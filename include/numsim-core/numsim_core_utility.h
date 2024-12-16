#ifndef UVWBASE_UTILITY_H
#define UVWBASE_UTILITY_H

#include <utility>
#include <any>
#include <functional>
#include <ostream>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <iomanip>

namespace numsim_core {

template <class T, typename... Args, class F>
inline std::pair<const std::type_index,
                 std::function<void(std::any const &, Args...)>>
to_any_visitor(F const &f) {
  return {std::type_index(typeid(T)),
          [g = f](std::any const &a, Args &&...args) {
            if constexpr (std::is_void_v<T>)
              g();
            else
              g(std::any_cast<T const &>(a), std::forward<Args>(args)...);
          }};
}

namespace detail {
template <class AlwaysVoid, template<class...> class Op, class... Args>
struct detector {
  using value_t = std::false_type;
};

template <template<class...> class Op, class... Args>
struct detector<std::void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
};
} // namespace detail

template <template<class...> class Op, class... Args>
using is_detected = typename detail::detector<void, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
const auto is_detected_v = is_detected<Op, Args...>::value;
}

#endif // UVWBASE_UTILITY_H
