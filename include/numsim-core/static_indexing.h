#ifndef STATIC_INDEXING_H
#define STATIC_INDEXING_H


#include <utility>

namespace numsim_core {

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
  static inline type_id value{static_indexing_inc<Derived>::getID()};
};

}

/// class for counting the different implementations
template <typename Derived, typename Base> class static_indexing : public Base {
public:
  using expr_type = Base;
  template <typename... Args>
  static_indexing(Args &&...args) : Base(std::forward<Args>(args)...) {}
  static_indexing() = default;
  static_indexing(static_indexing const &) = delete;
  static_indexing(static_indexing &&) = delete;
  virtual ~static_indexing() = default;
  const static_indexing &operator=(static_indexing const &) = delete;
  [[nodiscard]] type_id get_type_id() const noexcept final override {
    return m_id;
  }
  [[nodiscard]] static type_id get_static_type_id() noexcept {
    return m_id;
  }

private:
  static inline auto m_id{detail::static_indexing_imp<Derived>::value};
};


}

#endif // STATIC_INDEXING_H
