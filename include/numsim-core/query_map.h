#ifndef QUERY_MAP_H
#define QUERY_MAP_H

#include <type_traits>
#include <any>
#include <functional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace numsim_core {

/**
 * @brief A template class that maps a list of keys to values, allowing for
 * flexible querying and type erasure.
 *
 * This class provides a way to create a mapping between a set of keys and their
 * associated values, using a specified map implementation. The keys are
 * provided as a tuple, and the values can be retrieved or set using a
 * type-erased approach. The class supports querying with custom functions.
 *
 * @tparam List The tuple type representing the keys.
 * @tparam Map A template class for the map implementation (e.g.,
 * std::unordered_map).
 * @tparam TypeErasure The type used for type erasure, defaulting to std::any.
 */
template <typename List, template <class... ArgsMap> class Map,
          typename TypeErasure = std::any>
class query_map {
private:
  /**
   * @brief A private struct for handling the data mapping from the provided
   * List type.
   *
   * @tparam _List The tuple type used to extract key-value mappings.
   */
  template <typename _List> struct query_map_data;

  /**
   * @brief A specialization of query_map_data for tuples with multiple keys.
   *
   * @tparam _List The tuple type containing the keys.
   * @tparam _First The first key type.
   * @tparam _Keys The remaining key types.
   */
  template <template <typename... Args> class _List, typename _First,
            typename... _Keys>
  struct query_map_data<_List<_First, _Keys...>> {
    using tuple_type = std::tuple<_First, _Keys...>; ///< The type representing
                                                     ///< the keys as a tuple.
    using map_type =
        Map<_First, typename query_map_data<std::tuple<_Keys...>>::
                        map_type>; ///< The map type for key-value pairs.
  };

  /**
   * @brief A specialization of query_map_data for a tuple with a single key.
   *
   * @tparam _List The tuple type containing the key.
   * @tparam _First The key type.
   */
  template <template <typename... Args> class _List, typename _First>
  struct query_map_data<_List<_First>> {
    using tuple_type =
        std::tuple<_First>; ///< The type representing the key as a tuple.
    using map_type = Map<_First, TypeErasure>; ///< The map type for key-value
                                               ///< pairs with type erasure.
  };

public:
  /**
   * @brief The type of the map used to store values.
   */
  using map_type = typename query_map_data<List>::map_type;

  /**
   * @brief The type of the tuple representing the keys.
   */
  using tuple_type = typename query_map_data<List>::tuple_type;

  /**
   * @brief The type used for type erasure.
   */
  using type_erasure_type = TypeErasure;

  /**
   * @brief The signature for query functions, which take a reference to
   * type-erased data.
   */
  using query_fun_sig = std::function<void(type_erasure_type &)>;

  /**
   * @brief Retrieves the key type at the specified index.
   *
   * @tparam Index The index of the key type to retrieve.
   */
  template <std::size_t Index>
  using key_type = decltype(std::get<Index>(List()));

  /**
   * @brief The index sequence for iterating over keys in the tuple.
   */
  using index_sequence =
      std::make_index_sequence<std::tuple_size_v<tuple_type>>;

  /**
   * @brief Default constructor for query_map.
   */
  query_map() {}

  /**
   * @brief Sets a value in the map with the specified keys.
   *
   * This function takes a data reference, a function for value generation, and
   * a variadic list of keys.
   *
   * @tparam T The type of the data to be stored.
   * @tparam Func The type of the function used to generate the value.
   * @tparam Keys The types of the keys used to access the map.
   * @param data The data reference to be stored.
   * @param func The function to generate the value.
   * @param keys The keys to associate with the value.
   */
  template <typename T, typename Func, typename... Keys>
  void set(T &data, Func &&func, Keys &&...keys) {
    set_imp(m_data, data, std::forward<Func>(func),
            std::forward<Keys>(keys)...);
  }

  /**
   * @brief Retrieves a value from the map using the specified keys.
   *
   * This overload is non-const and allows for modifications to the retrieved
   * value.
   *
   * @tparam Keys The types of the keys used to access the map.
   * @param keys The keys used to retrieve the value.
   * @return A reference to the retrieved value.
   */
  template <typename... Keys> auto &get(Keys &&...keys) {
    return get_list_first(std::make_tuple(std::forward<Keys>(keys)...),
                          index_sequence{});
  }

  /**
   * @brief Retrieves a value from the map using the specified keys.
   *
   * This overload is const and does not allow modifications to the retrieved
   * value.
   *
   * @tparam Keys The types of the keys used to access the map.
   * @param keys The keys used to retrieve the value.
   * @return A const reference to the retrieved value.
   */
  template <typename... Keys> auto const &get(Keys &&...keys) const {
    return get_list_first(std::make_tuple(std::forward<Keys>(keys)...),
                          index_sequence{});
  }

  /**
   * @brief Adds a query function to be executed later.
   *
   * The query function is stored along with the keys that will be used to
   * retrieve values when the function is executed.
   *
   * @tparam Keys The types of the keys associated with the query.
   * @param func The function to execute during the query.
   * @param keys The keys to associate with the query.
   */
  template <typename... Keys> void query(query_fun_sig &&func, Keys &&...keys) {
    m_queries.emplace_back(
        std::make_pair(std::forward<query_fun_sig>(func),
                       std::make_tuple(std::forward<Keys>(keys)...)));
  }

  /**
   * @brief Executes all stored query functions using the current map data.
   */
  void final_queries() {
    for (auto &[func, id] : m_queries) {
      func(get_list_first(id, index_sequence{}));
    }
  }

private:
  /**
   * @brief Retrieves the first value from the map using the provided keys.
   *
   * This is a helper function for the `get` methods.
   *
   * @tparam _List The list type of keys.
   * @tparam First The index of the first key.
   * @tparam Seq The remaining indices of keys.
   * @param key_list The list of keys.
   * @return A reference to the retrieved value.
   * @throws std::invalid_argument if the key is not found in the map.
   */
  template <typename _List, std::size_t First, std::size_t... Seq>
  auto &get_list_first(_List const &key_list,
                       std::index_sequence<First, Seq...>) {
    const auto &key{std::get<First>(key_list)};
    if (m_data.find(key) == m_data.end()) {
      const auto str{[&key]() {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(key)>,
                                      std::string>) {
          return key;
        } else {
          return std::to_string(key);
        }
      }};
      throw std::invalid_argument(std::string("key ") + str() + std::string(" not found"));
    }
    return get_list_impl(m_data[key], key_list, std::index_sequence<Seq...>{});
  }

  /**
   * @brief Retrieves a value from the map using the provided keys and the last
   * index.
   *
   * @tparam DataMap The type of the data map.
   * @tparam _List The list type of keys.
   * @tparam Last The last index of keys.
   * @param map The data map to retrieve values from.
   * @param key_list The list of keys.
   * @return A reference to the retrieved value.
   * @throws std::invalid_argument if the key is not found in the map.
   */
  template <typename DataMap, typename _List, std::size_t Last>
  auto &get_list_impl(DataMap &map, _List const &key_list,
                      std::index_sequence<Last>) {
    const auto &key{std::get<Last>(key_list)};
    if (map.find(key) == map.end()) {
      const auto str{[&key]() {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(key)>,
                                      std::string>) {
          return key;
        } else {
          return std::to_string(key);
        }
      }};
      throw std::invalid_argument(std::string("key ") + str() + std::string(" not found"));
    }
    return map[key];
  }

  /**
   * @brief Base case for the recursive implementation of value retrieval from
   * the map.
   *
   * @tparam DataMap The type of the data map.
   * @tparam _List The list type of keys.
   * @param map The data map to retrieve values from.
   * @param key_list The list of keys (unused).
   * @return A reference to the map.
   */
  template <typename DataMap, typename _List>
  auto &get_list_impl(DataMap &map, _List const & /*key_list*/,
                      std::index_sequence<>) {
    return map;
  }

  /**
   * @brief Recursive helper function to set a value in the map using the
   * provided keys.
   *
   * @tparam MapPart The type of the current map being processed.
   * @tparam Data The type of the data to be stored.
   * @tparam Func The type of the function used to generate the value.
   * @tparam First The type of the first key.
   * @tparam Keys The remaining key types.
   * @param map The current map being processed.
   * @param data The data reference to be stored.
   * @param func The function to generate the value.
   * @param first The first key.
   * @param keys The remaining keys.
   */
  template <typename MapPart, typename Data, typename Func, typename First,
            typename... Keys>
  void set_imp(MapPart &map, Data &data, Func &&func, First &&first,
               Keys &&...keys) {
    set_imp(map[first], data, std::forward<Func>(func), keys...);
  }

  /**
   * @brief Base case for setting a value in the map using the last key.
   *
   * @tparam MapPart The type of the current map being processed.
   * @tparam T The type of the data to be stored.
   * @tparam Func The type of the function used to generate the value.
   * @tparam Last The type of the last key.
   * @param map The current map being processed.
   * @param data The data reference to be stored.
   * @param func The function to generate the value.
   * @param last The last key.
   */
  template <typename MapPart, typename T, typename Func, typename Last>
  void set_imp(MapPart &map, T &data, Func &&func, Last &&last) {
    map[last] = func(data);
  }

  map_type m_data{}; ///< The internal map that holds the key-value pairs.
  std::vector<std::pair<query_fun_sig, tuple_type>>
      m_queries{}; ///< A collection of queries to be executed later.
};

/**
 * @brief An example specialization of query_map using std::tuple as keys and
 * std::unordered_map as the underlying map type.
 */
using double_query_map =
    query_map<std::tuple<std::string, std::string>, std::unordered_map>;



//#include <gtest/gtest.h>
//#include <tuple>
//#include <any>
//#include <unordered_map>
//#include <string>
//#include <functional>

//// Assuming the query_map class is implemented as per your description above

//// Define test data
//using key_list = std::tuple<int, std::string>;
//using map_type = std::unordered_map<int, std::unordered_map<std::string, std::any>>;

//// Create a simple value generation function
//auto value_gen = [](int &data) -> std::any {
//  return std::make_any<int>(data);
//};

//// Test suite for the query_map class
//class QueryMapTest : public ::testing::Test {
//protected:
//  query_map<key_list, std::unordered_map> qmap;  // Object under test
//};

//// Test setting and getting values
//TEST_F(QueryMapTest, SetAndGetValues) {
//  int data = 42;

//  // Setting value with keys 1 and "key1"
//  qmap.set(data, value_gen, 1, std::string("key1"));

//  // Retrieving the stored value
//  auto &retrievedValue = std::any_cast<int&>(qmap.get(1, std::string("key1")));

//  // Assert that the retrieved value matches the original
//  EXPECT_EQ(retrievedValue, data);
//}

//// Test retrieval of values using const and non-const overloads
//TEST_F(QueryMapTest, ConstAndNonConstGet) {
//  int data = 100;

//  // Setting value with keys 2 and "key2"
//  qmap.set(data, value_gen, 2, std::string("key2"));

//         // Test non-const get
//  auto &nonConstValue = std::any_cast<int&>(qmap.get(2, std::string("key2")));
//  EXPECT_EQ(nonConstValue, data);

//         // Test const get
//  const auto &constValue = std::any_cast<const int&>(qmap.get(2, std::string("key2")));
//  EXPECT_EQ(constValue, data);
//}

//// Test behavior when a key is not found
//TEST_F(QueryMapTest, KeyNotFound) {
//  // Attempt to get value with non-existent key combination
//  EXPECT_THROW(qmap.get(3, std::string("nonexistent")), std::invalid_argument);
//}

//// Test storing and executing queries
//TEST_F(QueryMapTest, QueryExecution) {
//  int data = 55;
//  bool query_executed = false;

//         // Set a value with keys 1 and "key3"
//  qmap.set(data, value_gen, 1, std::string("key3"));

//         // Store a query that checks if the value matches the original
//  qmap.query([&query_executed](std::any &value) {
//    int retrieved_value = std::any_cast<int>(value);
//    EXPECT_EQ(retrieved_value, 55);
//    query_executed = true;
//  }, 1, std::string("key3"));

//         // Execute all stored queries
//  qmap.final_queries();

//         // Ensure the query was executed and passed
//  EXPECT_TRUE(query_executed);
//}

//// Test updating values
//TEST_F(QueryMapTest, UpdateValue) {
//  int initial_data = 10;
//  int updated_data = 99;

//         // Set an initial value
//  qmap.set(initial_data, value_gen, 4, std::string("key4"));

//  // Update the value using the same keys
//  qmap.set(updated_data, value_gen, 4, std::string("key4"));

//  // Retrieve and check the updated value
//  auto &retrievedValue = std::any_cast<int&>(qmap.get(4, std::string("key4")));
//  EXPECT_EQ(retrievedValue, updated_data);
//}

} // namespace uvwCommon

#endif // QUERY_MAP_H
