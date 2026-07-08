#include <gtest/gtest.h>

#include <numsim-core/property_graph/history_property.h>

#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

using numsim_core::history_property;
using numsim_core::property_traits;

// ── (1) Trivially-copyable value: raw-byte serialize/deserialize round-trips ──
TEST(HistoryPropertySerialize, TriviallyCopyableRawBytes) {
  history_property<double> h(1.5, 2.5, property_traits{});
  std::ostringstream os;
  h.serialize(os);

  history_property<double> g(0.0, 0.0, property_traits{});
  std::istringstream is(os.str());
  g.deserialize(is);
  EXPECT_DOUBLE_EQ(g.old_value(), 1.5);
  EXPECT_DOUBLE_EQ(g.new_value(), 2.5);
}

// A NON-trivially-copyable value type that provides the ADL hooks.
namespace hooked {
struct Payload {
  std::vector<int> data;
};
inline void numsim_serialize(std::ostream &os, Payload const &o,
                             Payload const &n) {
  os << o.data.size() << ' ';
  for (int x : o.data) os << x << ' ';
  os << n.data.size() << ' ';
  for (int x : n.data) os << x << ' ';
}
inline void numsim_deserialize(std::istream &is, Payload &o, Payload &n) {
  std::size_t k;
  is >> k;
  o.data.resize(k);
  for (auto &x : o.data) is >> x;
  is >> k;
  n.data.resize(k);
  for (auto &x : n.data) is >> x;
}
} // namespace hooked

// ── (2) Non-trivial value WITH an ADL hook: the hook is detected and used ─────
TEST(HistoryPropertySerialize, NonTrivialWithAdlHook) {
  static_assert(!std::is_trivially_copyable_v<hooked::Payload>);
  static_assert(numsim_core::has_numsim_serialize<hooked::Payload>::value);
  static_assert(numsim_core::has_numsim_deserialize<hooked::Payload>::value);

  history_property<hooked::Payload> h(hooked::Payload{{1, 2, 3}},
                                      hooked::Payload{{4, 5}},
                                      property_traits{});
  std::ostringstream os;
  h.serialize(os); // uses numsim_serialize via ADL
  // Payload is non-trivially-copyable, so the raw-byte branch is if-constexpr-
  // discarded and the hook branch is provably taken; a silently-empty hook
  // would still "round-trip" empty vectors, so pin that bytes were produced.
  EXPECT_FALSE(os.str().empty());

  history_property<hooked::Payload> g(hooked::Payload{}, hooked::Payload{},
                                      property_traits{});
  std::istringstream is(os.str());
  g.deserialize(is);
  EXPECT_EQ(g.old_value().data, (std::vector<int>{1, 2, 3}));
  EXPECT_EQ(g.new_value().data, (std::vector<int>{4, 5}));
}

// A NON-trivially-copyable value type with NO serialization hook.
struct NoHook {
  std::vector<int> data;
};

// ── (3) THE REGRESSION FIX: a non-trivially-copyable type WITHOUT a hook must
// still be storable as history — instantiation, commit/revert and clone work;
// only serialize/deserialize fail, and only at call time (never at instantiation
// — the previous static_assert-in-virtual made the class uninstantiable). ──────
TEST(HistoryPropertySerialize, NonTrivialWithoutHookIsStorable) {
  static_assert(!std::is_trivially_copyable_v<NoHook>);
  static_assert(!numsim_core::has_numsim_serialize<NoHook>::value);

  history_property<NoHook> h(NoHook{{1}}, NoHook{{2, 3}}, property_traits{});

  // commit()/revert() use operator= — valid for any assignable T.
  h.commit();
  EXPECT_EQ(h.old_value().data, (std::vector<int>{2, 3}));
  h.new_value().data = {9};
  h.revert();
  EXPECT_EQ(h.new_value().data, (std::vector<int>{2, 3}));

  // clone() copy-constructs a deep, independent, correctly-typed copy.
  auto c = h.clone();
  ASSERT_NE(c, nullptr);
  auto *d = dynamic_cast<history_property<NoHook> *>(c.get());
  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d->old_value().data, h.old_value().data);
  EXPECT_EQ(d->new_value().data, h.new_value().data);
  h.new_value().data = {42}; // mutate the source …
  EXPECT_EQ(d->new_value().data, (std::vector<int>{2, 3})); // … clone unaffected

  // Serialization is the ONLY operation that requires a hook; without one it
  // throws at call time rather than blocking instantiation. Match on the
  // message so a wrong-path throw can't satisfy the assertion.
  auto throws_with = [](auto &&fn, const char *needle) {
    try {
      fn();
    } catch (const std::runtime_error &e) {
      return std::string(e.what()).find(needle) != std::string::npos;
    } catch (...) {
      return false;
    }
    return false;
  };
  std::ostringstream os;
  EXPECT_TRUE(throws_with([&] { h.serialize(os); }, "numsim_serialize"));
  std::istringstream is("ignored");
  EXPECT_TRUE(throws_with([&] { h.deserialize(is); }, "numsim_deserialize"));
}

// ── Contract notes (compile-time-enforced, so not runtime tests) ─────────────
// The dispatch now rejects two silent-correctness hazards at COMPILE time, so
// they cannot be expressed as runtime tests (they would fail to build):
//   • a trivially-copyable T that ALSO defines a numsim_serialize hook
//     (ambiguous — the raw-byte path would silently win); and
//   • a type defining exactly ONE of numsim_serialize / numsim_deserialize
//     (you could write state you can never read back).
// Both are guarded by static_asserts inside the taken if-constexpr branches of
// serialize()/deserialize(); a type with NEITHER hook still compiles (it takes
// the throwing else-branch), which is what NonTrivialWithoutHookIsStorable pins.
