#ifndef STATIC_INDEXING_H
#define STATIC_INDEXING_H

#include "namespace.h"


#include <utility>

namespace numsim::core {

using type_id = unsigned int;

namespace detail {
template<typename Derived>
struct static_indexing_inc
{
  static inline type_id m_id{0};
  static inline type_id m_max_id{0};

  static type_id getID() {
    m_max_id++;
    return m_id++;
  }
};

template <typename Type, typename Derived = void>
struct static_indexing_imp {
  // The id is drawn on first use, from a function-local static, so it is
  // valid whenever it is read. Inline static data members of class
  // template specialisations have unordered dynamic initialisation: an
  // initialiser that copied `value` directly could run before `value`'s
  // own and read 0 (clang does this), giving every type the same id.
  [[nodiscard]] static type_id get() {
    static const type_id id{static_indexing_inc<Derived>::getID()};
    return id;
  }
  static inline type_id value{get()};
};

}

/// class for counting the different implementations
template <typename Derived, typename Base> class static_indexing : public Base {
public:
  using expr_type = Base;
  template <typename... Args>
  static_indexing(Args &&...args) : Base(std::forward<Args>(args)...) {}
  static_indexing() = default;
  //static_indexing(static_indexing const &) = delete;
  //static_indexing(static_indexing &&) = delete;
  virtual ~static_indexing() = default;
  const static_indexing &operator=(static_indexing const &) = delete;
//  [[nodiscard]] type_id get_type_id() const noexcept final override {
//    return m_id;
//  }
  /// Id of `Derived`; safe to call at any time, including from other
  /// static initialisers.
  [[nodiscard]] static type_id get_static_type_id() {
    return detail::static_indexing_imp<Derived>::get();
  }

//private:
  // Initialised through get(), never by copying another inline static, so
  // it holds the right id once its own initialiser has run.
  static inline auto m_id{detail::static_indexing_imp<Derived>::get()};
};


}

#endif // STATIC_INDEXING_H
