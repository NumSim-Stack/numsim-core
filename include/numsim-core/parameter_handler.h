#ifndef PARAMETER_HANDLER_H
#define PARAMETER_HANDLER_H

#include "any_printer.h"
#include <any>
#include <stdexcept>
#include <string>
#include <unordered_map>



namespace numsim_core {

template<typename T>
struct parameter_handler_cast;

template<>
struct parameter_handler_cast<std::any>{
  template<typename T>
  constexpr inline T operator()(std::any & data)const{
    return std::any_cast<T>(data);
  }

  template<typename T>
  constexpr inline T operator()(std::any const& data)const{
    return std::any_cast<T>(data);
  }
};

/**
 * @brief A class for handling parameters with dynamic typing.
 *
 * This class allows for storing and retrieving parameters of varying types
 * using a key-value mechanism. It provides type-safe access to stored
 * parameters through templates, along with functions to check for existence,
 * print contents, and clear stored parameters.
 *
 * @tparam KeyType Type of the keys used for storing parameters (default is
 * std::string).
 * @tparam TypeErasure Type used for the values stored in the handler (default
 * is std::any).
 */
template <typename KeyType = std::string, typename TypeErasure = std::any>
class parameter_handler {
public:
  using key_type =
      KeyType; ///< Alias for the key type used in the parameter handler.
  using cast = parameter_handler_cast<TypeErasure>;

  /**
   * @brief Constructs an empty parameter handler.
   */
  parameter_handler() {}

  parameter_handler(parameter_handler const& data):m_data(data.m_data) {}

  /**
   * @brief Inserts or assigns a value to the specified key.
   *
   * This function will insert the key-value pair into the parameter handler.
   * If the key already exists, its value will be updated.
   *
   * @tparam T The type of the value being inserted.
   * @param name The key under which the value is stored.
   * @param value The value to be inserted.
   * @return A reference to the inserted or updated value.
   */
  template <typename T> T &insert(KeyType &&name, T &&value) {
    auto iter{m_data.insert_or_assign(std::move(name), std::move(value))};
    return cast().template operator()<T &>(iter.first->second);
  }

  /**
   * @brief Inserts or assigns a value to the specified key (const overload).
   *
   * This function behaves the same as the non-const version but takes a
   * constant reference for the key.
   *
   * @tparam T The type of the value being inserted.
   * @param name The key under which the value is stored.
   * @param value The value to be inserted.
   * @return A reference to the inserted or updated value.
   */
  template <typename T> T &insert(KeyType const &name, T &&value) {
    auto iter{m_data.insert_or_assign(name, std::move(value))};
    return cast().template operator()<T &>(iter.first->second);
  }

  /**
   * @brief Inserts or assigns a value to the specified key.
   *
   * This function will insert the key-value pair into the parameter handler.
   * If the key already exists, its value will be updated.
   *
   * @tparam T The type of the value being inserted.
   * @param name The key under which the value is stored.
   * @param value The value to be inserted.
   * @return A reference to the inserted or updated value.
   */
  template <typename T> T &insert(KeyType &&name, T const&value) {
    auto iter{m_data.insert_or_assign(std::move(name), value)};
    return cast().template operator()<T &>(iter.first->second);
  }

  /**
   * @brief Inserts or assigns a value to the specified key (const overload).
   *
   * This function behaves the same as the non-const version but takes a
   * constant reference for the key.
   *
   * @tparam T The type of the value being inserted.
   * @param name The key under which the value is stored.
   * @param value The value to be inserted.
   * @return A reference to the inserted or updated value.
   */
  template <typename T> T &insert(KeyType const &name, T const&value) {
    auto iter{m_data.insert_or_assign(name, value)};
    return cast().template operator()<T &>(iter.first->second);
  }

  /**
   * @brief Retrieves a value associated with the specified key.
   *
   * This function will throw an exception if the key is not found.
   *
   * @tparam T The type of the value being retrieved.
   * @param name The key for which the value is to be retrieved.
   * @return A const reference to the value associated with the key.
   * @throws std::invalid_argument if the key is not found.
   */
  template <typename T> const T &get(KeyType &&name) const {
    auto pos{m_data.find(name)};
    if (pos == m_data.end()) {
      throw std::invalid_argument("Key " + name + " not found");
    }
    return cast().template operator()<T const&>(pos->second);
  }

  /**
   * @brief Retrieves a value associated with the specified key (const
   * overload).
   *
   * This function behaves the same as the non-const version but takes a
   * constant reference for the key.
   *
   * @tparam T The type of the value being retrieved.
   * @param name The key for which the value is to be retrieved.
   * @return A const reference to the value associated with the key.
   * @throws std::invalid_argument if the key is not found.
   */
  template <typename T> const T &get(KeyType const &name) const {
    auto pos{m_data.find(name)};
    if (pos == m_data.end()) {
      throw std::invalid_argument("Key " + name + " not found");
    }
    return cast().template operator()<T const&>(pos->second);
  }

  /**
   * @brief Retrieves a value associated with the specified key.
   *
   * This function will throw an exception if the key is not found.
   *
   * @tparam T The type of the value being retrieved.
   * @param name The key for which the value is to be retrieved.
   * @return A const reference to the value associated with the key.
   * @throws std::invalid_argument if the key is not found.
   */
  template <typename T> T &get(KeyType &&name) {
    auto pos{m_data.find(name)};
    if (pos == m_data.end()) {
      throw std::invalid_argument("Key " + name + " not found");
    }
    return cast().template operator()<T &>(pos->second);
  }

  /**
   * @brief Retrieves a value associated with the specified key (const
   * overload).
   *
   * This function behaves the same as the non-const version but takes a
   * constant reference for the key.
   *
   * @tparam T The type of the value being retrieved.
   * @param name The key for which the value is to be retrieved.
   * @return A const reference to the value associated with the key.
   * @throws std::invalid_argument if the key is not found.
   */
  template <typename T> T &get(KeyType const &name) {
    auto pos{m_data.find(name)};
    if (pos == m_data.end()) {
      throw std::invalid_argument("Key " + name + " not found");
    }
    return cast().template operator()<T &>(pos->second);
  }

  /**
   * @brief Retrieves the type-erased value associated with the specified key.
   *
   * This function will throw an exception if the key is not found.
   *
   * @param name The key for which the value is to be retrieved.
   * @return A const reference to the type-erased value associated with the key.
   * @throws std::invalid_argument if the key is not found.
   */
  const TypeErasure &data(KeyType &&name) const {
    auto pos{m_data.find(name)};
    if (pos == m_data.end()) {
      throw std::invalid_argument("Key " + name + " not found");
    }
    return pos->second;
  }

  /**
   * @brief Retrieves the type-erased value associated with the specified key
   * (const overload).
   *
   * This function behaves the same as the non-const version but takes a
   * constant reference for the key.
   *
   * @param name The key for which the value is to be retrieved.
   * @return A const reference to the type-erased value associated with the key.
   * @throws std::invalid_argument if the key is not found.
   */
  const TypeErasure &data(KeyType const &name) const {
    auto pos{m_data.find(name)};
    if (pos == m_data.end()) {
      throw std::invalid_argument("Key " + name + " not found");
    }
    return pos->second;
  }

  /**
   * @brief Checks if a key exists in the parameter handler.
   *
   * @param name The key to check for existence.
   * @return True if the key exists; false otherwise.
   */
  auto contains(KeyType &&name) const {
    return m_data.find(name) != m_data.end();
  }

  /**
   * @brief Checks if a key exists in the parameter handler (const overload).
   *
   * This function behaves the same as the non-const version but takes a
   * constant reference for the key.
   *
   * @param name The key to check for existence.
   * @return True if the key exists; false otherwise.
   */
  auto contains(KeyType const &name) const {
    return m_data.find(name) != m_data.end();
  }

  /**
   * @brief Prints all key-value pairs stored in the parameter handler.
   *
   * @param os The output stream where the data will be printed.
   */
  void print(std::ostream &os) {
    for (const auto &[name, type] : m_data) {
      os << name << ": ";
      os << ::print(type)
         << "\n"; // Assuming a global print function exists for TypeErasure
    }
  }

  /**
   * @brief Clears all key-value pairs from the parameter handler.
   */
  void clear() { m_data.clear(); }

private:
  std::unordered_map<KeyType, TypeErasure>
      m_data; ///< Internal storage for key-value pairs.
};

} // namespace numsim_core
#endif // PARAMETER_HANDLER_H
