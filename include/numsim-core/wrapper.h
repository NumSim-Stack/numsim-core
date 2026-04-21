#ifndef WRAPPER_H
#define WRAPPER_H

#include <any>
#include <type_traits>

namespace numsim_core {

/**
 * @brief A lightweight wrapper class template for managing a pointer to an
 * object of type T.
 *
 * This class provides a simple wrapper around a pointer to an object of type
 * `T`, allowing access and manipulation of the data through dereferencing
 * operators and methods. It ensures that the pointer is always valid and
 * provides mechanisms for safely obtaining a reference to the underlying
 * object, both in `const` and mutable contexts.
 *
 * @tparam T The type of the object to be wrapped. Can be a const-qualified or
 * non-const type.
 */
template <typename T> class wrapper {
public:
  /**
   * @brief Defines a type alias for the underlying type, stripping
   * const-qualification from T.
   */
  using type = std::remove_const_t<T>;

  /**
   * @brief Default constructor. Initializes the wrapper with a null pointer.
   *
   * The object will not hold a valid pointer until assigned.
   */
  constexpr wrapper() noexcept = default;

  /**
   * @brief Constructs a wrapper from a reference to an object of type T.
   *
   * @param data A reference to an object of type T to be wrapped.
   * The wrapper will hold a pointer to this object.
   */
  constexpr explicit wrapper(T &data) noexcept : m_data(&data) {}

  /**
   * @brief Copy constructor. Constructs a wrapper from another wrapper.
   *
   * @param data The wrapper to be copied.
   * The newly constructed wrapper will hold the same pointer as the original.
   */
  constexpr explicit wrapper(wrapper const &data) noexcept
      : m_data(data.m_data) {}

  /**
   * @brief Destructor. Cleans up the wrapper.
   *
   * The destructor does not delete the pointer, as ownership is not transferred
   * or managed.
   */
  ~wrapper() noexcept = default;

  /**
   * @brief Copy assignment operator.
   *
   * Assigns the contents of one wrapper to another, copying the pointer.
   *
   * @param data The wrapper to copy from.
   * @return A reference to this wrapper after assignment.
   */
  constexpr inline const auto &operator=(wrapper const &data) noexcept {
    m_data = data.m_data;
    return *this;
  }

  /**
   * @brief Dereference operator.
   *
   * Allows access to the underlying object through dereferencing.
   *
   * @return A const reference to the underlying object.
   */
  constexpr inline const T &operator*() const noexcept { return *m_data; }

  /**
   * @brief Retrieves the underlying object, optionally casting it to a
   * different type.
   *
   * This method allows access to the underlying object, potentially casting it
   * to a specified type.
   *
   * @tparam Type The type to cast the object to, defaults to T.
   * @return A const reference to the underlying object.
   */
  template <typename Type = T> constexpr inline const T &get() const noexcept {
    return *static_cast<Type *>(m_data);
  }

  /**
   * @brief Retrieves the underlying object, optionally casting it to a
   * different type.
   *
   * This method allows mutable access to the underlying object, potentially
   * casting it to a specified type.
   *
   * @tparam Type The type to cast the object to, defaults to T.
   * @return A reference to the underlying object.
   */
  template <typename Type = T,
            std::enable_if_t<!std::is_const_v<Type>, bool> = true>
  constexpr inline T &get() noexcept {
    return *static_cast<Type *>(m_data);
  }

  /**
   * @brief Boolean conversion operator.
   *
   * Allows the wrapper to be used in boolean contexts. The result is `true` if
   * the wrapped pointer is not null, otherwise `false`.
   *
   * @return `true` if the pointer is not null, `false` otherwise.
   */
  constexpr inline operator bool() const noexcept { return m_data != nullptr; }

private:
  /**
   * @brief Pointer to the wrapped object.
   *
   * This pointer is used to reference the underlying object of type T.
   * It is initialized to `nullptr` by default.
   */
  T *m_data{nullptr};
};

template <typename T> using cwrapper = wrapper<const T>;

} // namespace numsim_core
#endif // WRAPPER_H
