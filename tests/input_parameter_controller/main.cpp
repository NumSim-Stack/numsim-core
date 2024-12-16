#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <numsim-core/parameter_handler.h>

using numsim_core::parameter_handler;

// Test fixture class
class ParameterHandlerTest : public ::testing::Test {
protected:
  parameter_handler<> handler;  // Default key and type-erasure types
};

// Test for inserting and retrieving an integer value
TEST_F(ParameterHandlerTest, InsertAndRetrieveInt) {
  int value = 42;
  handler.insert("key1", value);
  EXPECT_EQ(handler.get<int>("key1"), value);
}

// Test for inserting and retrieving a string value
TEST_F(ParameterHandlerTest, InsertAndRetrieveString) {
  const std::string value = "Hello, World!";
  handler.insert("key2", value);
  EXPECT_EQ(handler.get<std::string>("key2"), value);
}

// Test for exception when retrieving a non-existing key
TEST_F(ParameterHandlerTest, GetThrowsWhenKeyNotFound) {
  EXPECT_THROW(handler.get<int>("non_existent_key"), std::invalid_argument);
}

// Test for checking if a key exists using `contains`
TEST_F(ParameterHandlerTest, ContainsKey) {
  handler.insert("key3", 123);
  EXPECT_TRUE(handler.contains("key3"));
  EXPECT_FALSE(handler.contains("non_existent_key"));
}

// Test for retrieving data using `data()` method
TEST_F(ParameterHandlerTest, RetrieveDataAsTypeErasure) {
  int value = 10;
  handler.insert("key4", value);
  const std::any& retrievedData = handler.data("key4");
  EXPECT_EQ(std::any_cast<int>(retrievedData), value);
}

// Test for clearing the handler
TEST_F(ParameterHandlerTest, ClearData) {
  handler.insert("key5", 100);
  handler.clear();
  EXPECT_FALSE(handler.contains("key5"));
}

// Test for inserting by moving key and value
TEST_F(ParameterHandlerTest, InsertMoveKeyAndValue) {
  std::string key = "key6";
  std::string value = "movable_string";
  handler.insert(std::move(key), std::move(value));
  EXPECT_EQ(handler.get<std::string>("key6"), "movable_string");
}

// Test for exception in `data()` when key is not found
TEST_F(ParameterHandlerTest, DataThrowsWhenKeyNotFound) {
  EXPECT_THROW(handler.data("non_existent_key"), std::invalid_argument);
}

// Test for printing data to an output stream
TEST_F(ParameterHandlerTest, PrintContents) {
  handler.insert("key7", std::string("print_test"));
  std::ostringstream oss;
  handler.print(oss);
  EXPECT_NE(oss.str().find("key7"), std::string::npos);
  EXPECT_NE(oss.str().find("print_test"), std::string::npos);
}
