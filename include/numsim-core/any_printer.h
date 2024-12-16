#ifndef ANY_PRINTER_H
#define ANY_PRINTER_H

#include "numsim_core_utility.h"

namespace numsim_core {

/**
 * @brief A utility class that provides type-safe printing of `std::any` types.
 *
 * This class contains a static map `any_print_visitor` that stores visitors for
 * various types. Each type is associated with a `std::function` that specifies
 * how to print the contained data in the `std::any` object. This allows for
 * safe, type-erased printing of various types by looking up the appropriate
 * visitor function for the contained type.
 *
 * @note The class supports adding new types by adding additional visitors using
 * the `to_any_visitor` function template.
 *
 * @see std::any
 */
class any_print_wrapper {
  /**
   * @brief A static map that holds type-erased printing functions for various
   * types.
   *
   * This map stores `std::function` objects that accept a `std::any` object and
   * an `std::ostream&`. It enables safe printing of `std::any` values based on
   * their type. The key is a `std::type_index` corresponding to the type
   * contained in the `std::any` object, and the value is the function that
   * knows how to print that type to the stream.
   */
  static inline const std::unordered_map<
      std::type_index, std::function<void(std::any const &, std::ostream &)>>
      any_print_visitor{
          /**
           * @brief Visitor for printing `int` values from a `std::any` object.
           *
           * Prints the integer value contained in the `std::any` to the given
           * output stream.
           */
          to_any_visitor<int, std::ostream &>(
              [](int x, std::ostream &os) { os << x; }),

          /**
           * @brief Visitor for printing `unsigned` values from a `std::any`
           * object.
           *
           * Prints the unsigned integer value contained in the `std::any` to
           * the given output stream.
           */
          to_any_visitor<unsigned, std::ostream &>(
              [](unsigned x, std::ostream &os) { os << x; }),

          /**
           * @brief Visitor for printing `float` values from a `std::any`
           * object.
           *
           * Prints the floating-point value contained in the `std::any` to the
           * given output stream.
           */
          to_any_visitor<float, std::ostream &>(
              [](float x, std::ostream &os) { os << x; }),

          /**
           * @brief Visitor for printing `double` values from a `std::any`
           * object.
           *
           * Prints the double-precision floating-point value contained in the
           * `std::any` to the given output stream.
           */
          to_any_visitor<double, std::ostream &>(
              [](double x, std::ostream &os) { os << x; }),

          /**
           * @brief Visitor for printing `std::string` values from a `std::any`
           * object.
           *
           * Prints the string contained in the `std::any` to the given output
           * stream.
           */
          to_any_visitor<std::string, std::ostream &>(
              [](std::string const &x, std::ostream &os) { os << x; }),

          /**
           * @brief Visitor for printing `std::vector<std::string>` values from
           * a `std::any` object.
           *
           * Prints the elements of the vector of strings contained in the
           * `std::any` to the given output stream.
           */
          to_any_visitor<std::vector<std::string>, std::ostream &>(
              [](std::vector<std::string> const &x, std::ostream &os) {
                for (const auto &entry : x) {
                  os << entry << " ";
                }
              }),

          /**
           * @brief Visitor for printing C-string values (`char const*`) from a
           * `std::any` object.
           *
           * Prints the C-string contained in the `std::any` to the given output
           * stream.
           */
          to_any_visitor<char const *, std::ostream &>(
              [](char const *s, std::ostream &os) { os << std::quoted(s); }),

          // Additional types
          /**
           * @brief Visitor for printing `bool` values from a `std::any` object.
           *
           * Prints the boolean value contained in the `std::any` to the given
           * output stream as "true" or "false".
           */
          to_any_visitor<bool, std::ostream &>(
              [](bool x, std::ostream &os) { os << (x ? "true" : "false"); }),

          /**
           * @brief Visitor for printing `long` values from a `std::any` object.
           *
           * Prints the long integer value contained in the `std::any` to the
           * given output stream.
           */
          to_any_visitor<long, std::ostream &>(
              [](long x, std::ostream &os) { os << x; }),

          /**
           * @brief Visitor for printing `std::vector<int>` values from a
           * `std::any` object.
           *
           * Prints the elements of the vector of integers contained in the
           * `std::any` to the given output stream.
           */
          to_any_visitor<std::vector<int>, std::ostream &>(
              [](std::vector<int> const &x, std::ostream &os) {
                for (const auto &entry : x) {
                  os << entry << " ";
                }
              }),

          /**
           * @brief Visitor for printing `std::vector<double>` values from a
           * `std::any` object.
           *
           * Prints the elements of the vector of doubles contained in the
           * `std::any` to the given output stream.
           */
          to_any_visitor<std::vector<double>, std::ostream &>(
              [](std::vector<double> const &x, std::ostream &os) {
                for (const auto &entry : x) {
                  os << entry << " ";
                }
              }),

          /**
           * @brief Visitor for printing `std::tuple<int, double, std::string>`
           * values from a `std::any` object.
           *
           * Prints the elements of the tuple `(int, double, std::string)`
           * contained in the `std::any` to the given output stream.
           */
          to_any_visitor<std::tuple<int, double, std::string>, std::ostream &>(
              [](std::tuple<int, double, std::string> const &t,
                 std::ostream &os) {
                os << "(" << std::get<0>(t) << ", " << std::get<1>(t) << ", "
                   << std::quoted(std::get<2>(t)) << ")";
              })};

public:
  /**
   * @brief Constructs an `any_print_wrapper` object with a reference to the
   * given `std::any` data.
   *
   * @param data The `std::any` object to be printed.
   */
  explicit any_print_wrapper(std::any const &data) : m_data(data) {}

  /**
   * @brief Overloads the stream insertion operator to print the wrapped
   * `std::any` data.
   *
   * This function finds the correct visitor function for the type contained in
   * `std::any` and prints the value to the output stream.
   *
   * @param os The output stream.
   * @param data The `any_print_wrapper` object containing the `std::any` data.
   * @return std::ostream& The output stream after printing.
   * @throws std::runtime_error If the type contained in `std::any` is not found
   * in the `any_print_visitor`.
   */
  friend std::ostream &operator<<(std::ostream &os, any_print_wrapper data) {
    auto pos = any_print_visitor.find(std::type_index(data.m_data.type()));
    if (pos == any_print_visitor.end()) {
      throw std::runtime_error(
          "type id " + std::string(data.m_data.type().name()) + " not found\n");
    }
    pos->second(data.m_data, os);
    return os;
  }

private:
  std::any const &m_data; ///< The `std::any` object being printed.
};

} // namespace uvwCommon

inline auto print(std::any const &data) {
  return numsim_core::any_print_wrapper(data);
}


//#include <gtest/gtest.h>
//#include <any>
//#include <iostream>
//#include <sstream>
//#include <string>
//#include <vector>
//#include <tuple>

//// Include your any_print_wrapper class here
//// #include "any_print_wrapper.hpp"

//class AnyPrintWrapperTest : public ::testing::Test {
//protected:
//  // Utility function to capture printed output
//  std::string capture_output(std::any const &data) {
//    std::ostringstream oss;
//    any_print_wrapper wrapped_data(data);
//    oss << wrapped_data;
//    return oss.str();
//  }
//};

//// Test printing an int
//TEST_F(AnyPrintWrapperTest, PrintInt) {
//  std::any data = 42;
//  EXPECT_EQ(capture_output(data), "42");
//}

//// Test printing an unsigned
//TEST_F(AnyPrintWrapperTest, PrintUnsigned) {
//  std::any data = 123u;
//  EXPECT_EQ(capture_output(data), "123");
//}

//// Test printing a float
//TEST_F(AnyPrintWrapperTest, PrintFloat) {
//  std::any data = 3.14f;
//  EXPECT_EQ(capture_output(data), "3.14");
//}

//// Test printing a double
//TEST_F(AnyPrintWrapperTest, PrintDouble) {
//  std::any data = 2.71828;
//  EXPECT_EQ(capture_output(data), "2.71828");
//}

//// Test printing a std::string
//TEST_F(AnyPrintWrapperTest, PrintString) {
//  std::any data = std::string("hello");
//  EXPECT_EQ(capture_output(data), "hello");
//}

//// Test printing a vector of strings
//TEST_F(AnyPrintWrapperTest, PrintVectorString) {
//  std::any data = std::vector<std::string>{"apple", "banana", "cherry"};
//  EXPECT_EQ(capture_output(data), "apple banana cherry ");
//}

//// Test printing a C-string
//TEST_F(AnyPrintWrapperTest, PrintCString) {
//  std::any data = "c-style string";
//  EXPECT_EQ(capture_output(data), "\"c-style string\"");
//}

//// Test printing a bool
//TEST_F(AnyPrintWrapperTest, PrintBool) {
//  std::any data = true;
//  EXPECT_EQ(capture_output(data), "true");
//}

//// Test printing a long
//TEST_F(AnyPrintWrapperTest, PrintLong) {
//  std::any data = 1234567890L;
//  EXPECT_EQ(capture_output(data), "1234567890");
//}

//// Test printing a vector of ints
//TEST_F(AnyPrintWrapperTest, PrintVectorInt) {
//  std::any data = std::vector<int>{1, 2, 3, 4};
//  EXPECT_EQ(capture_output(data), "1 2 3 4 ");
//}

//// Test printing a vector of doubles
//TEST_F(AnyPrintWrapperTest, PrintVectorDouble) {
//  std::any data = std::vector<double>{1.1, 2.2, 3.3};
//  EXPECT_EQ(capture_output(data), "1.1 2.2 3.3 ");
//}

//// Test printing a tuple<int, double, std::string>
//TEST_F(AnyPrintWrapperTest, PrintTuple) {
//  std::any data = std::make_tuple(10, 3.14, std::string("tuple test"));
//  EXPECT_EQ(capture_output(data), "(10, 3.14, \"tuple test\")");
//}

//// Test for an unsupported type (should throw an exception)
//TEST_F(AnyPrintWrapperTest, UnsupportedTypeThrowsException) {
//  std::any data = std::vector<bool>{true, false};
//  EXPECT_THROW(capture_output(data), std::runtime_error);
//}

//// Test for an empty std::any (should throw an exception)
//TEST_F(AnyPrintWrapperTest, EmptyAnyThrowsException) {
//  std::any data;
//  EXPECT_THROW(capture_output(data), std::runtime_error);
//}


#endif // ANY_PRINTER_H
