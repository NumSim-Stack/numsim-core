#ifndef NUMSIM_CORE_PG_HISTORY_PROPERTY_H
#define NUMSIM_CORE_PG_HISTORY_PROPERTY_H

#include <istream>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <numsim-core/property_graph/property.h>

namespace numsim_core {

template<typename T, typename Traits>
class history_property;

// ── Serialization customization point ───────────────────────────────────────
// A history_property byte-serializes trivially-copyable values directly. For a
// value type that is NOT trivially copyable (e.g. a tmech::tensor, which is
// copyable+assignable but carries user-provided special members, or any type
// with heap state), provide free functions found by ADL in the type's namespace:
//
//   void numsim_serialize(std::ostream&, T const& old_v, T const& new_v);
//   void numsim_deserialize(std::istream&, T& old_v, T& new_v);
//
// Whether a hook exists is a property of the SERIALIZE operation, not of being
// storable as history — so its absence is a *runtime* error only if serialize()
// is actually called, never a compile-time barrier to instantiating
// history_property<T>. (The previous design enforced trivial-copyability via a
// static_assert in the virtual serialize() body, which the compiler evaluates
// when the class specialization is completed — making history_property<T>
// uninstantiable for any non-trivially-copyable T even when never serialized,
// and diverging across compilers per [temp.inst]/11 on virtual instantiation.)
template <typename T, typename = void>
struct has_numsim_serialize : std::false_type {};
template <typename T>
struct has_numsim_serialize<
    T, std::void_t<decltype(numsim_serialize(
           std::declval<std::ostream &>(), std::declval<T const &>(),
           std::declval<T const &>()))>> : std::true_type {};

template <typename T, typename = void>
struct has_numsim_deserialize : std::false_type {};
template <typename T>
struct has_numsim_deserialize<
    T, std::void_t<decltype(numsim_deserialize(
           std::declval<std::istream &>(), std::declval<T &>(),
           std::declval<T &>()))>> : std::true_type {};

template<typename T, typename Traits>
auto make_history_property(T&& old_value, T&& new_value, Traits&& traits) {
  using value_t = std::decay_t<T>;
  using traits_t = std::decay_t<Traits>;
  return std::make_unique<history_property<value_t, traits_t>>(
      std::forward<T>(old_value), std::forward<T>(new_value), std::forward<Traits>(traits));
}

template<typename T, typename Traits = property_traits>
class history_property final : public property_base<Traits> {
public:
  using base = property_base<Traits>;

  history_property(T&& old_value, T&& new_value, Traits&& traits)
      : base(std::forward<Traits>(traits)), m_old(std::move(old_value)), m_new(std::move(new_value)) {}
  history_property(const T& old_value, const T& new_value, const Traits& traits)
      : base(traits), m_old(old_value), m_new(new_value) {}

  virtual property_base_ptr<Traits> clone() const override {
    return make_history_property(m_old, m_new, base::m_traits);
  }

  T& old_value() { return m_old; }
  T& new_value() { return m_new; }
  const T& old_value() const { return m_old; }
  const T& new_value() const { return m_new; }

  static_assert(!std::is_reference_v<T>,
                "history_property does not support reference types");

  bool is_history() const noexcept override { return true; }
  void commit() noexcept override { m_old = m_new; }
  void revert() noexcept override { m_new = m_old; }

  /// Serialize old and new values. Trivially-copyable types are written as raw
  /// bytes; otherwise a `numsim_serialize` ADL hook is used (see the
  /// customization point above). A non-trivially-copyable type without a hook
  /// throws here — but instantiating history_property<T> never requires either.
  void serialize(std::ostream& os) const override {
    if constexpr (std::is_trivially_copyable_v<T>) {
      os.write(reinterpret_cast<const char*>(&m_old), sizeof(T));
      os.write(reinterpret_cast<const char*>(&m_new), sizeof(T));
    } else if constexpr (has_numsim_serialize<T>::value) {
      numsim_serialize(os, m_old, m_new); // ADL
    } else {
      throw std::runtime_error(
          "history_property::serialize: value type is not trivially copyable "
          "and has no numsim_serialize(std::ostream&, T const&, T const&) "
          "overload reachable by ADL");
    }
  }

  void deserialize(std::istream& is) override {
    if constexpr (std::is_trivially_copyable_v<T>) {
      is.read(reinterpret_cast<char*>(&m_old), sizeof(T));
      is.read(reinterpret_cast<char*>(&m_new), sizeof(T));
    } else if constexpr (has_numsim_deserialize<T>::value) {
      numsim_deserialize(is, m_old, m_new); // ADL
    } else {
      throw std::runtime_error(
          "history_property::deserialize: value type is not trivially copyable "
          "and has no numsim_deserialize(std::istream&, T&, T&) overload "
          "reachable by ADL");
    }
  }

private:
  T m_old;
  T m_new;
};

} // namespace numsim_core

#endif // NUMSIM_CORE_PG_HISTORY_PROPERTY_H
