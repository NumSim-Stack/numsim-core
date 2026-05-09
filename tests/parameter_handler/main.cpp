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
  // The check catches std::bad_any_cast internally and rethrows as
  // std::invalid_argument with a parameter-named diagnostic — strictly
  // more useful than the bare bad_any_cast which carries no context.
  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
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

// ---------------------------------------------------------------------------
// Policy hints: range<Min, Max> and unit_label<"...">
// ---------------------------------------------------------------------------

using numsim_core::range;
using numsim_core::range_hint_base;
using numsim_core::unit_label;
using numsim_core::units_hint_base;

TEST_F(InputParameterTest, TestRangePolicyAcceptsInBoundsValue) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<range<1, 4096>>();

  handler.insert("nx", 1024);

  EXPECT_NO_THROW(paramController.check_parameter(handler));
}

TEST_F(InputParameterTest, TestRangePolicyRejectsBelowMin) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<range<1, 4096>>();

  handler.insert("nx", 0);

  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}

TEST_F(InputParameterTest, TestRangePolicyRejectsAboveMax) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<range<1, 4096>>();

  handler.insert("nx", 5000);

  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}

TEST_F(InputParameterTest, TestRangePolicyExposesBoundsViaSideBase) {
  // GUI introspection: walk the parameter's checks and dynamic_cast each
  // to range_hint_base. The first match yields the bounds as std::any
  // wrapping the parameter's underlying type.
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<range<1, 4096>>();

  range_hint_base const* hint = nullptr;
  for (auto const& check : param.checks()) {
    if (auto const* h = dynamic_cast<range_hint_base const*>(check.get())) {
      hint = h;
      break;
    }
  }
  ASSERT_NE(hint, nullptr);
  EXPECT_EQ(std::any_cast<int>(hint->min_value()), 1);
  EXPECT_EQ(std::any_cast<int>(hint->max_value()), 4096);
}

TEST_F(InputParameterTest, TestRangePolicyDoesNotFireWhenParameterMissing) {
  // Like check_range, the policy is silent for absent parameters —
  // is_required is the policy that catches missing values. Two-policy
  // composition lets the schema express "must be present AND in range"
  // explicitly without conflating the concerns.
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<range<1, 4096>>();

  EXPECT_NO_THROW(paramController.check_parameter(handler));
}

TEST_F(InputParameterTest, TestUnitLabelPolicyExposesUnits) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<unit_label<"cells">>();

  units_hint_base const* hint = nullptr;
  for (auto const& check : param.checks()) {
    if (auto const* h = dynamic_cast<units_hint_base const*>(check.get())) {
      hint = h;
      break;
    }
  }
  ASSERT_NE(hint, nullptr);
  EXPECT_EQ(hint->units(), std::string_view{"cells"});
}

TEST_F(InputParameterTest, TestUnitLabelPolicyHasNoRuntimeCheck) {
  // Pure metadata — neither presence nor any value should trigger an
  // exception from the unit_label policy alone.
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<unit_label<"cells">>();

  EXPECT_NO_THROW(paramController.check_parameter(handler));   // missing
  handler.insert("nx", -42);
  EXPECT_NO_THROW(paramController.check_parameter(handler));   // present, any value
}

TEST_F(InputParameterTest, TestPolicyCompositionWithExistingChecks) {
  // is_required + range + unit_label compose. Each carries its own
  // concern; they don't interfere.
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<is_required>();
  param.add<range<1, 4096>>();
  param.add<unit_label<"cells">>();

  handler.insert("nx", 256);

  EXPECT_NO_THROW(paramController.check_parameter(handler));

  // Verify both side-bases are reachable.
  bool found_range = false, found_units = false;
  for (auto const& check : param.checks()) {
    if (dynamic_cast<range_hint_base const*>(check.get())) found_range = true;
    if (dynamic_cast<units_hint_base const*>(check.get())) found_units = true;
  }
  EXPECT_TRUE(found_range);
  EXPECT_TRUE(found_units);
}

TEST_F(InputParameterTest, TestRangePolicyWithDoubleBounds) {
  // Bounds match the parameter's underlying type via static_cast — for a
  // double parameter, write the literals as 0.0 / 1.0 so the NTTP types
  // line up.
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<double>("target_fraction");
  param.add<range<0.0, 1.0>>();

  handler.insert("target_fraction", 0.25);
  EXPECT_NO_THROW(paramController.check_parameter(handler));

  handler.insert("target_fraction", 1.5);
  EXPECT_THROW(paramController.check_parameter(handler), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// description_label policy
// ---------------------------------------------------------------------------

using numsim_core::description_hint_base;
using numsim_core::description_label;

TEST_F(InputParameterTest, TestDescriptionLabelExposesText) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<description_label<"voxel-grid resolution along x">>();

  description_hint_base const* hint = nullptr;
  for (auto const& check : param.checks()) {
    if (auto const* h = dynamic_cast<description_hint_base const*>(check.get())) {
      hint = h;
      break;
    }
  }
  ASSERT_NE(hint, nullptr);
  EXPECT_EQ(hint->description_text(),
            std::string_view{"voxel-grid resolution along x"});
}

TEST_F(InputParameterTest, TestDescriptionLabelHasNoRuntimeCheck) {
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<description_label<"a description">>();

  EXPECT_NO_THROW(paramController.check_parameter(handler));   // missing
  handler.insert("nx", -1);
  EXPECT_NO_THROW(paramController.check_parameter(handler));   // present, any value
}

TEST_F(InputParameterTest, TestFullPolicyComposition) {
  // The four canonical policies compose: validation (is_required +
  // range) plus pure metadata (unit_label + description_label). Each
  // side-base is independently introspectable from the GUI side.
  input_parameter_controller<std::string, MockParameterHandler> paramController;
  auto &param = paramController.insert<int>("nx");
  param.add<is_required>();
  param.add<range<1, 4096>>();
  param.add<unit_label<"cells">>();
  param.add<description_label<"voxel-grid resolution along x">>();

  handler.insert("nx", 256);
  EXPECT_NO_THROW(paramController.check_parameter(handler));

  bool found_range = false, found_units = false, found_desc = false;
  for (auto const& check : param.checks()) {
    if (dynamic_cast<range_hint_base const*>(check.get())) found_range = true;
    if (dynamic_cast<units_hint_base const*>(check.get())) found_units = true;
    if (dynamic_cast<description_hint_base const*>(check.get())) found_desc = true;
  }
  EXPECT_TRUE(found_range);
  EXPECT_TRUE(found_units);
  EXPECT_TRUE(found_desc);
}
