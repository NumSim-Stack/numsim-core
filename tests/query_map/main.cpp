#include <gtest/gtest.h>
#include <tuple>
#include <any>
#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>
#include <numsim-core/query_map.h>

using numsim_core::query_map;

// Assuming the query_map class is implemented as per your description above

// Define test data
using key_list = std::tuple<int, std::string>;
using map_type = std::unordered_map<int, std::unordered_map<std::string, std::any>>;

// Create a simple value generation function
auto value_gen = [](int &data) -> std::any {
  return std::make_any<int>(data);
};

// Test suite for the query_map class
class QueryMapTest : public ::testing::Test {
protected:
  query_map<key_list, std::unordered_map> qmap;  // Object under test
};

// Test setting and getting values
TEST_F(QueryMapTest, SetAndGetValues) {
  int data = 42;

         // Setting value with keys 1 and "key1"
  qmap.set(data, value_gen, 1, std::string("key1"));

         // Retrieving the stored value
  auto &retrievedValue = std::any_cast<int&>(qmap.get(1, std::string("key1")));

         // Assert that the retrieved value matches the original
  EXPECT_EQ(retrievedValue, data);
}

// Test retrieval of values using const and non-const overloads
TEST_F(QueryMapTest, ConstAndNonConstGet) {
  int data = 100;

         // Setting value with keys 2 and "key2"
  qmap.set(data, value_gen, 2, std::string("key2"));

         // Test non-const get
  auto &nonConstValue = std::any_cast<int&>(qmap.get(2, std::string("key2")));
  EXPECT_EQ(nonConstValue, data);

         // Test const get
  const auto &constValue = std::any_cast<const int&>(qmap.get(2, std::string("key2")));
  EXPECT_EQ(constValue, data);
}

// Test behavior when a key is not found
TEST_F(QueryMapTest, KeyNotFound) {
  // Attempt to get value with non-existent key combination
  EXPECT_THROW(qmap.get(3, std::string("nonexistent")), std::invalid_argument);
}

// Test storing and executing queries
TEST_F(QueryMapTest, QueryExecution) {
  int data = 55;
  bool query_executed = false;

         // Set a value with keys 1 and "key3"
  qmap.set(data, value_gen, 1, std::string("key3"));

         // Store a query that checks if the value matches the original
  qmap.query([&query_executed](std::any &value) {
    int retrieved_value = std::any_cast<int>(value);
    EXPECT_EQ(retrieved_value, 55);
    query_executed = true;
  }, 1, std::string("key3"));

         // Execute all stored queries
  qmap.final_queries();

         // Ensure the query was executed and passed
  EXPECT_TRUE(query_executed);
}

// Test updating values
TEST_F(QueryMapTest, UpdateValue) {
  int initial_data = 10;
  int updated_data = 99;

         // Set an initial value
  qmap.set(initial_data, value_gen, 4, std::string("key4"));

         // Update the value using the same keys
  qmap.set(updated_data, value_gen, 4, std::string("key4"));

         // Retrieve and check the updated value
  auto &retrievedValue = std::any_cast<int&>(qmap.get(4, std::string("key4")));
  EXPECT_EQ(retrievedValue, updated_data);
}
