#ifndef PARAMETER_HANDLER_H
#define PARAMETER_HANDLER_H

#include "any_printer.h"
#include <any>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace numsim_core {

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

  /**
   * @brief Constructs an empty parameter handler.
   */
  parameter_handler() {}

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
    return std::any_cast<T &>(iter.first->second);
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
    return std::any_cast<T &>(iter.first->second);
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
    return std::any_cast<T &>(iter.first->second);
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
    return std::any_cast<T &>(iter.first->second);
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
    return std::any_cast<const T &>((pos->second));
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
    return std::any_cast<const T &>((pos->second));
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
    return std::any_cast<const T &>((pos->second));
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
    return std::any_cast<const T &>((pos->second));
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

//#include <gtest/gtest.h>
//#include <any>
//#include <string>
//#include <unordered_map>
//#include <stdexcept>
//#include "parameter_handler.h"  // Include the file with the parameter_handler class

//// Test fixture class
//class ParameterHandlerTest : public ::testing::Test {
//protected:
//  parameter_handler<> handler;
//};

//// Test for inserting and retrieving an integer value
//TEST_F(ParameterHandlerTest, InsertAndRetrieveInt) {
//  int value = 42;
//  handler.insert("key1", value);

//  EXPECT_EQ(handler.get<int>("key1"), value);
//}

//// Test for inserting and retrieving a string value
//TEST_F(ParameterHandlerTest, InsertAndRetrieveString) {
//  std::string value = "Hello";
//  handler.insert("key2", value);

//  EXPECT_EQ(handler.get<std::string>("key2"), value);
//}

//// Test for exception when retrieving a non-existing key
//TEST_F(ParameterHandlerTest, GetThrowsWhenKeyNotFound) {
//  EXPECT_THROW(handler.get<int>("non_existent_key"), std::invalid_argument);
//}

//// Test for checking if a key exists using contains
//TEST_F(ParameterHandlerTest, Contains) {
//  handler.insert("key3", 123);

//  EXPECT_TRUE(handler.contains("key3"));
//  EXPECT_FALSE(handler.contains("non_existent_key"));
//}

//// Test for retrieving data using the data() method
//TEST_F(ParameterHandlerTest, RetrieveDataAsTypeErasure) {
//  int value = 10;
//  handler.insert("key4", value);

//  const std::any &retrievedData = handler.data("key4");
//  EXPECT_EQ(std::any_cast<int>(retrievedData), value);
//}

//// Test for clearing the handler
//TEST_F(ParameterHandlerTest, ClearData) {
//  handler.insert("key5", 100);
//  handler.clear();

//  EXPECT_FALSE(handler.contains("key5"));
//}

//// Test for inserting by moving key and value
//TEST_F(ParameterHandlerTest, InsertMove) {
//  std::string key = "key6";
//  std::string value = "movable_string";

//  handler.insert(std::move(key), std::move(value));

//  EXPECT_EQ(handler.get<std::string>("key6"), "movable_string");
//}

//// Test for exception in data() when key is not found
//TEST_F(ParameterHandlerTest, DataThrowsWhenKeyNotFound) {
//  EXPECT_THROW(handler.data("non_existent_key"), std::invalid_argument);
//}



} // namespace numsim_core
#endif // PARAMETER_HANDLER_H
