#include "numsim-core/input_parameter_controller.h"
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

using numsim_core::check_data_type;
using numsim_core::check_range;
using numsim_core::input_parameter_controller;
using numsim_core::is_required;
using numsim_core::set_default;

// Mock implementation of ParameterHandler for tests
class MockParameterHandler {
public:
  bool contains(const std::string &name) const {
    return parameters.find(name) != parameters.end();
  }

  template <typename T> T get(const std::string &name) const {
    return std::any_cast<T>(parameters.at(name));
  }

  template <typename T> void insert(const std::string &name, T value) {
    parameters[name] = value;
  }

private:
  std::unordered_map<std::string, std::any> parameters;
};

// Unit tests for input_parameter and its checks
class InputParameterTest : public ::testing::Test {
protected:
  MockParameterHandler handler;
};

TEST_F(InputParameterTest, TestRequiredParameterPresent) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("test_param");
  param.add<is_required>();

  // Insert required parameter into the handler
  handler.insert("test_param", 42);

  // Should not throw exception as parameter is present
  EXPECT_NO_THROW(paramController.check_parameter(handler));
}

TEST_F(InputParameterTest, TestRequiredParameterMissing) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("missing_param");
  param.add<is_required>();

  // Parameter not inserted, should throw an exception
  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}

TEST_F(InputParameterTest, TestParameterInRange) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("range_param");
  param.add<check_range>(0, 100);

  handler.insert("range_param", 50); // In-range value

  // Should not throw exception as value is within range
  EXPECT_NO_THROW(paramController.check_parameter(handler));
}

TEST_F(InputParameterTest, TestParameterOutOfRange) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("range_param");
  param.add<check_range>(0, 100);

  handler.insert("range_param", 150); // Out of range value

  // Should throw exception as value is out of range
  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}

TEST_F(InputParameterTest, TestSetDefaultValue) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("default_param");
  param.add<set_default>(99); // Default value

  // Parameter is missing, so default value should be inserted
  paramController.check_parameter(handler);

  EXPECT_EQ(handler.get<int>("default_param"),
            99); // Default value should be set
}

TEST_F(InputParameterTest, TestSetDefaultValueNotNeeded) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("default_param");
  param.add<set_default>(99); // Default value

  handler.insert("default_param",
                 42); // Insert a value that overrides the default

  paramController.check_parameter(handler);

  EXPECT_EQ(handler.get<int>("default_param"),
            42); // Default value should not be set
}

TEST_F(InputParameterTest, TestDefaultValueInsertion) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<std::string>("param_with_default");
  param.add<set_default>("default_value");

  // Since the parameter is not inserted, the default value should be inserted
  paramController.check_parameter(handler);

  EXPECT_EQ(handler.get<std::string>("param_with_default"), "default_value");
}

TEST_F(InputParameterTest, TestExistingParameterDoesNotOverrideDefault) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<std::string>("param_with_default");
  param.add<set_default>("default_value");

  // Insert a non-default value for the parameter
  handler.insert("param_with_default", std::string("user_value"));

  paramController.check_parameter(handler);

  EXPECT_EQ(handler.get<std::string>("param_with_default"), "user_value");
}

TEST_F(InputParameterTest, TestMultipleChecks) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;

  // Add multiple checks: required, range, and default value
  auto &param = paramController.insert<int>("multi_param");
  param.add<set_default>(50);
  param.add<is_required>();
  param.add<check_range>(0, 100);

  // Test: Missing parameter, should insert default value
  paramController.check_parameter(handler);
  EXPECT_EQ(handler.get<int>("multi_param"), 50);

  // Test: Parameter present and in range, should not throw
  handler.insert<int>("multi_param", 75);
  EXPECT_NO_THROW(paramController.check_parameter(handler));

  // Test: Parameter present but out of range, should throw
  handler.insert<int>("multi_param", 150);
  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}

TEST_F(InputParameterTest, TestInvalidType) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("param_with_wrong_type");
  param.add<check_data_type>();

  handler.insert("param_with_wrong_type", std::string("not_an_int"));
  EXPECT_THROW(paramController.check_parameter(handler), std::bad_any_cast);
}

TEST_F(InputParameterTest, TestRequiredParameterMissing_Fail) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("missing_param");
  param.add<is_required>();

  // Do not insert the required parameter, so this should throw an exception
  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}

TEST_F(InputParameterTest, TestRangeCheck_Fail) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("range_param");
  param.add<check_range>(0, 100); // Range: 0 to 100

  // Insert a value outside the range (greater than 100)
  handler.insert("range_param", 150);

  // Expect failure: this should throw an exception
  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}

TEST_F(InputParameterTest, TestDefaultValueOverride_Fail) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("param_with_default");
  param.add<set_default>(99); // Default value: 99

  // Insert a different value first
  handler.insert("param_with_default", 42);

  paramController.check_parameter(handler);

  // The value should still be 42, as the default was not supposed to be applied
  // after insertion.
  EXPECT_EQ(handler.get<int>("param_with_default"), 42);
}

TEST_F(InputParameterTest, TestMultipleChecks_Fail) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;

  // Add multiple checks: required, range, and default value
  auto &param = paramController.insert<int>("multi_param");
  param.add<is_required>();
  param.add<check_range>(0, 100);
  param.add<set_default>(50);

  // Test: Insert a value outside of the valid range (e.g., 150)
  handler.insert("multi_param", 150);

  // This should fail the test because the value is out of the allowed range
  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}
