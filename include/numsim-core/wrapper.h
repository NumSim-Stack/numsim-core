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

//// Test suite for wrapper class
// class WrapperTest : public ::testing::Test
//{
// protected:
//   int value = 42; // Regular int to be wrapped
//   const int constValue = 100; // Const int to test const wrapper
// };

//// Test default constructor and null pointer state
// TEST_F(WrapperTest, DefaultConstructor)
//{
//   wrapper<int> w;
//   EXPECT_FALSE(w);  // The wrapper should not contain a valid pointer
// }

//// Test explicit constructor that wraps an object
// TEST_F(WrapperTest, WrapsObject)
//{
//   wrapper<int> w(value);
//   EXPECT_TRUE(w);  // The wrapper should contain a valid pointer
//   EXPECT_EQ(*w, value); // Dereferencing the wrapper should return the
//   original value
// }

//// Test const object wrapper
// TEST_F(WrapperTest, WrapsConstObject)
//{
//   wrapper<const int> w(constValue);
//   EXPECT_TRUE(w);  // The wrapper should contain a valid pointer
//   EXPECT_EQ(*w, constValue); // Dereferencing should return the const value
// }

//// Test the copy constructor
// TEST_F(WrapperTest, CopyConstructor)
//{
//   wrapper<int> w1(value);
//   wrapper<int> w2(w1);
//   EXPECT_EQ(*w2, value);  // w2 should hold the same value as w1
// }

//// Test copy assignment operator
// TEST_F(WrapperTest, CopyAssignment)
//{
//   wrapper<int> w1(value);
//   wrapper<int> w2;
//   w2 = w1;
//   EXPECT_EQ(*w2, value);  // w2 should now hold the same value as w1
// }

//// Test get() method for non-const wrapper
// TEST_F(WrapperTest, GetMutable)
//{
//   wrapper<int> w(value);
//   EXPECT_EQ(w.get(), value); // Should return the reference to the original
//   value w.get() = 50; // Modify the value through the wrapper
//   EXPECT_EQ(value, 50); // The original value should now be modified
// }

//// Test get() method for const wrapper
// TEST_F(WrapperTest, GetConst)
//{
//   wrapper<const int> w(constValue);
//   EXPECT_EQ(w.get(), constValue); // Should return the const reference to the
//   original value
// }

//// Test boolean conversion operator
// TEST_F(WrapperTest, BoolConversion)
//{
//   wrapper<int> w1;
//   wrapper<int> w2(value);

//  EXPECT_FALSE(w1); // w1 is empty, should convert to false
//  EXPECT_TRUE(w2);  // w2 contains a valid pointer, should convert to true
//}

//// Test assignment to a different wrapper object
// TEST_F(WrapperTest, Reassignment)
//{
//   int anotherValue = 123;
//   wrapper<int> w1(value);
//   wrapper<int> w2(anotherValue);
//   w2 = w1;
//   EXPECT_EQ(*w2, value); // After reassignment, w2 should point to the same
//   value as w1
// }

} // namespace numsim_core
#endif // WRAPPER_H
