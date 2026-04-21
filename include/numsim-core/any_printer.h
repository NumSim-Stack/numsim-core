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
  static const std::unordered_map<
      std::type_index, std::function<void(std::any const &, std::ostream &)>>
      any_print_visitor;

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

const std::unordered_map<
    std::type_index, std::function<void(std::any const &, std::ostream &)>> any_print_wrapper::any_print_visitor{
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
                                         }),

                                     to_any_visitor<std::reference_wrapper<const double>, std::ostream &>(
                                         [](std::reference_wrapper<const double> const &x, std::ostream &os) {
                                           os << x.get(); }),

                                     to_any_visitor<std::vector<std::any>, std::ostream &>(
                                         [](std::vector<std::any> const &x, std::ostream &os) {
                                           for (const auto &entry : x) {
                                             os << any_print_wrapper(entry) << " ";
                                           }
                                         }),

        to_any_visitor<std::vector<std::pair<std::any,std::any>>, std::ostream &>(
            [](std::vector<std::pair<std::any,std::any>> const &x, std::ostream &os) {
              for (const auto &[first, second] : x) {
                os << any_print_wrapper(first) << " " << any_print_wrapper(second) << " \n";
              }
            }),

                                     to_any_visitor<std::reference_wrapper<double>, std::ostream &>(
                                         [](std::reference_wrapper<double> const &x, std::ostream &os) {
                                           os << x.get(); })};


} // namespace numsim_core

numsim_core::any_print_wrapper print(std::any const &data) {
  return numsim_core::any_print_wrapper(data);
}

#endif // ANY_PRINTER_H
