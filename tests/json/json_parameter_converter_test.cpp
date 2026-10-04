// JSON object + parameter schema -> validated parameter_handler.

#include <numsim-core/input_parameter_controller.h>
#include <numsim-core/json/json_parameter_converter.h>
#include <numsim-core/parameter_handler.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace nc = numsim::core;
using json = nlohmann::json;
using handler = nc::parameter_handler<>;
using schema_type = nc::input_parameter_controller<std::string, handler>;

namespace {

schema_type solver_schema() {
  schema_type s;
  s.insert<double>("cg_tolerance").add<nc::is_required>();
  s.insert<std::size_t>("max_iterations").add<nc::set_default>(std::size_t{100});
  s.insert<std::string>("name").add<nc::is_required>();
  s.insert<std::vector<double>>("lengths");
  s.insert<bool>("verbose").add<nc::set_default>(false);
  return s;
}

std::string message_of(auto &&f) {
  try {
    f();
  } catch (std::exception const &e) {
    return e.what();
  }
  return {};
}

} // namespace

TEST(json_to_parameters, reads_typed_values_and_applies_defaults) {
  auto const doc = json::parse(R"({"type": "newton_cg", "name": "solver", "cg_tolerance": 1e-8,
                                   "lengths": [1.0, 2.0, 3.0]})");
  handler params;
  nc::json_to_parameters(doc, solver_schema(), params);
  EXPECT_DOUBLE_EQ(params.get<double>("cg_tolerance"), 1e-8);
  EXPECT_EQ(params.get<std::string>("name"), "solver");
  EXPECT_EQ(params.get<std::size_t>("max_iterations"), 100u);
  EXPECT_EQ(params.get<std::vector<double>>("lengths"), (std::vector<double>{1, 2, 3}));
  EXPECT_FALSE(params.get<bool>("verbose"));
}

TEST(json_to_parameters, missing_required_parameters_are_named_in_the_error_with_the_path) {
  auto const doc = json::parse(R"({"type": "newton_cg"})");
  handler params;
  auto const msg{message_of(
      [&] { nc::json_to_parameters(doc, solver_schema(), params, {.path = "objects[3]"}); })};
  EXPECT_NE(msg.find("objects[3]"), std::string::npos) << msg;
  EXPECT_NE(msg.find("cg_tolerance"), std::string::npos) << msg;
  EXPECT_NE(msg.find("name"), std::string::npos) << msg;
}

TEST(json_to_parameters, a_value_of_the_wrong_type_names_the_parameter_and_the_path) {
  auto const doc = json::parse(R"({"name": "solver", "cg_tolerance": "tight"})");
  handler params;
  auto const msg{
      message_of([&] { nc::json_to_parameters(doc, solver_schema(), params, {.path = "rve.solver"}); })};
  EXPECT_NE(msg.find("rve.solver.cg_tolerance"), std::string::npos) << msg;
}

TEST(json_to_parameters, unknown_keys_warn_by_default_and_are_errors_in_strict_mode) {
  auto const doc = json::parse(R"({"name": "solver", "cg_tolerance": 1e-8, "cg_tolerence": 1e-6})");
  handler lenient;
  testing::internal::CaptureStderr();
  EXPECT_NO_THROW(nc::json_to_parameters(doc, solver_schema(), lenient));
  EXPECT_NE(testing::internal::GetCapturedStderr().find("cg_tolerence"), std::string::npos);

  handler strict;
  auto const msg{message_of([&] {
    nc::json_to_parameters(doc, solver_schema(), strict,
                           {.unknown_keys = nc::unknown_key_policy::error, .path = "solver"});
  })};
  EXPECT_NE(msg.find("solver.cg_tolerence"), std::string::npos) << msg;

  handler quiet;
  testing::internal::CaptureStderr();
  EXPECT_NO_THROW(nc::json_to_parameters(doc, solver_schema(), quiet,
                                         {.unknown_keys = nc::unknown_key_policy::ignore}));
  EXPECT_EQ(testing::internal::GetCapturedStderr(), "");
}

TEST(json_to_parameters, keys_reserved_by_the_document_format_are_not_unknown) {
  auto const doc = json::parse(R"({"type": "x", "name": "solver", "cg_tolerance": 1e-8})");
  handler params;
  EXPECT_NO_THROW(nc::json_to_parameters(doc, solver_schema(), params,
                                         {.unknown_keys = nc::unknown_key_policy::error}));
}

TEST(json_reader_registry, custom_readers_extend_the_default_registry) {
  struct point {
    double x, y;
  };
  auto registry{nc::make_default_json_registry<json>()};
  registry.add<point>([](json const &j) -> std::any { return point{j.at(0).get<double>(), j.at(1).get<double>()}; });
  schema_type s;
  s.insert<point>("origin").add<nc::is_required>();
  handler params;
  nc::json_to_parameters(json::parse(R"({"origin": [1.5, -2]})"), s, params, registry);
  EXPECT_DOUBLE_EQ(params.get<point>("origin").y, -2.0);
}

TEST(json_reader_registry, a_type_without_reader_is_reported_with_its_parameter) {
  struct opaque {};
  schema_type s;
  s.insert<opaque>("blob");
  handler params;
  auto const msg{message_of([&] {
    nc::json_to_parameters(json::parse(R"({"blob": 1})"), s, params, {.path = "obj"});
  })};
  EXPECT_NE(msg.find("obj.blob"), std::string::npos) << msg;
}
