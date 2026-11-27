#include <numsim-core/static_indexing.h>

#include <gtest/gtest.h>

#include <set>

namespace {
struct base {
  virtual ~base() = default;
};

template <typename T> struct shape_a : numsim_core::static_indexing<shape_a<T>, base> {};
template <typename T> struct shape_b : numsim_core::static_indexing<shape_b<T>, base> {};
template <typename T> struct shape_c : numsim_core::static_indexing<shape_c<T>, base> {};

// Read during dynamic initialisation, before main: m_id copies of an
// unordered inline static used to see 0 here (clang), so every type
// shared id 0.
std::set<numsim_core::type_id> const ids_at_static_init{
    shape_a<double>::m_id, shape_b<double>::m_id, shape_c<double>::m_id,
    shape_a<float>::m_id};
} // namespace

TEST(static_indexing, ids_are_distinct_per_type) {
  std::set<numsim_core::type_id> const ids{
      shape_a<double>::m_id, shape_b<double>::m_id, shape_c<double>::m_id,
      shape_a<float>::m_id};
  EXPECT_EQ(ids.size(), 4u);
}

TEST(static_indexing, m_id_matches_get_static_type_id) {
  EXPECT_EQ(shape_a<double>::m_id, shape_a<double>::get_static_type_id());
  EXPECT_EQ(shape_b<double>::m_id, shape_b<double>::get_static_type_id());
  EXPECT_EQ(shape_c<double>::m_id, shape_c<double>::get_static_type_id());
  EXPECT_EQ(shape_a<float>::m_id, shape_a<float>::get_static_type_id());
}

TEST(static_indexing, ids_read_during_static_init_are_distinct) {
  EXPECT_EQ(ids_at_static_init.size(), 4u);
}
