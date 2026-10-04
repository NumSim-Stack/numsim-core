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

// --- Numbers are converted only when the value fits the parameter's type ---

namespace {

template <typename T> std::string read_error(char const *doc) {
  schema_type s;
  s.insert<T>("v");
  handler params;
  return message_of([&] { nc::json_to_parameters(json::parse(doc), s, params, {.path = "obj"}); });
}

template <typename T> T read_value(char const *doc) {
  schema_type s;
  s.insert<T>("v");
  handler params;
  nc::json_to_parameters(json::parse(doc), s, params);
  return params.get<T>("v");
}

} // namespace

TEST(json_numbers, a_fraction_is_not_an_integer) {
  EXPECT_NE(read_error<int>(R"({"v": 2.7})").find("obj.v"), std::string::npos);
  EXPECT_NE(read_error<std::size_t>(R"({"v": 3.9})").find("integer"), std::string::npos);
}

TEST(json_numbers, a_negative_value_is_not_an_unsigned_integer) {
  auto const msg{read_error<std::size_t>(R"({"v": -1})")};
  EXPECT_NE(msg.find("obj.v"), std::string::npos) << msg;
  EXPECT_NE(msg.find("range"), std::string::npos) << msg;
}

TEST(json_numbers, integers_out_of_range_are_rejected) {
  EXPECT_NE(read_error<int>(R"({"v": 1e12})").find("range"), std::string::npos);
  EXPECT_NE(read_error<int>(R"({"v": 3000000000})").find("range"), std::string::npos);
  EXPECT_NE(read_error<float>(R"({"v": 1e300})").find("range"), std::string::npos);
}

TEST(json_numbers, integral_values_written_as_floating_point_are_integers) {
  EXPECT_EQ(read_value<std::size_t>(R"({"v": 1e6})"), 1000000u);
  EXPECT_EQ(read_value<int>(R"({"v": -4.0})"), -4);
  EXPECT_DOUBLE_EQ(read_value<double>(R"({"v": 3})"), 3.0);
  EXPECT_FLOAT_EQ(read_value<float>(R"({"v": 0.5})"), 0.5f);
}

TEST(json_numbers, null_and_other_kinds_are_rejected) {
  EXPECT_NE(read_error<double>(R"({"v": null})").find("obj.v"), std::string::npos);
  EXPECT_NE(read_error<int>(R"({"v": "3"})").find("obj.v"), std::string::npos);
  EXPECT_NE(read_error<bool>(R"({"v": 1})").find("obj.v"), std::string::npos);
}

// --- Lists ---

TEST(json_lists, a_single_value_is_not_a_list) {
  EXPECT_NE(read_error<std::vector<std::size_t>>(R"({"v": 5})").find("array"), std::string::npos);
  EXPECT_NE(read_error<std::vector<double>>(R"({"v": 5})").find("array"), std::string::npos);
  EXPECT_NE(read_error<std::vector<std::string>>(R"({"v": "a"})").find("array"), std::string::npos);
}

TEST(json_lists, elements_are_checked_like_single_values_and_named_by_index) {
  auto const msg{read_error<std::vector<std::size_t>>(R"({"v": [1, -2]})")};
  EXPECT_NE(msg.find("obj.v[1]"), std::string::npos) << msg;
  EXPECT_EQ(read_value<std::vector<std::size_t>>(R"({"v": [1, 2e3]})"),
            (std::vector<std::size_t>{1, 2000}));
}

// --- Errors carry the path once, as a typed exception ---

TEST(json_conversion_error, validation_errors_carry_the_object_path) {
  schema_type s;
  s.insert<double>("poisson").add<nc::check_range>(-1.0, 0.5);
  handler params;
  try {
    nc::json_to_parameters(json::parse(R"({"poisson": 0.7})"), s, params, {.path = "materials[1]"});
    FAIL() << "no error";
  } catch (nc::json_conversion_error const &e) {
    EXPECT_EQ(e.path(), "materials[1]");
    EXPECT_EQ(std::string{e.what()}.rfind("materials[1]: ", 0), 0u) << e.what();
  }
}

TEST(json_conversion_error, reader_errors_carry_the_parameter_path_exactly_once) {
  handler params;
  try {
    nc::json_to_parameters(json::parse(R"({"name": "s", "cg_tolerance": "x"})"), solver_schema(), params,
                           {.path = "rve.solver"});
  } catch (nc::json_conversion_error const &e) {
    std::string const what{e.what()};
    EXPECT_EQ(e.path(), "rve.solver.cg_tolerance");
    EXPECT_EQ(what.find("rve.solver"), 0u) << what;
    EXPECT_EQ(what.find("rve.solver", 1), std::string::npos) << what;
    return;
  }
  FAIL() << "no error";
}

TEST(json_conversion_error, a_non_object_is_rejected_with_its_path) {
  handler params;
  try {
    nc::json_to_parameters(json::parse("[1, 2]"), solver_schema(), params, {.path = "objects[0]"});
    FAIL() << "no error";
  } catch (nc::json_conversion_error const &e) {
    EXPECT_EQ(e.path(), "objects[0]");
    EXPECT_NE(std::string{e.what()}.find("object"), std::string::npos);
  }
}

// --- Warnings can be collected instead of printed ---

TEST(json_to_parameters, unknown_key_warnings_go_to_the_callback_when_one_is_given) {
  std::vector<std::pair<std::string, std::string>> warnings;
  handler params;
  testing::internal::CaptureStderr();
  nc::json_to_parameters(json::parse(R"({"name": "s", "cg_tolerance": 1, "tol": 2})"), solver_schema(), params,
                         {.path = "solver", .on_warning = [&](std::string const &path, std::string const &msg) {
                            warnings.emplace_back(path, msg);
                          }});
  EXPECT_EQ(testing::internal::GetCapturedStderr(), "");
  ASSERT_EQ(warnings.size(), 1u);
  EXPECT_EQ(warnings[0].first, "solver.tol");
}

// --- Readers receive the parameter key; per-key readers win over per-type ones ---

TEST(json_reader_registry, readers_get_the_key_and_keyed_readers_take_precedence) {
  using pair = std::pair<std::string, std::string>;
  auto registry{nc::make_default_json_registry<json>()};
  registry.add<pair>([](json const &j, std::string const &key) -> std::any {
    if (!j.is_array() || j.size() != 2)
      throw std::invalid_argument(key + " must be a [row, column] pair");
    return pair{j[0].get<std::string>(), j[1].get<std::string>()};
  });
  registry.add_for_key<pair>("term", [](json const &j, std::string const &) -> std::any {
    return pair{j.at("block").get<std::string>(), "weighted"};
  });
  schema_type s;
  s.insert<pair>("zero_block");
  s.insert<pair>("term");
  handler params;
  nc::json_to_parameters(json::parse(R"({"zero_block": ["a", "b"], "term": {"block": "c"}})"), s, params,
                         registry);
  EXPECT_EQ(params.get<pair>("zero_block").second, "b");
  EXPECT_EQ(params.get<pair>("term").second, "weighted");

  handler bad;
  auto const msg{message_of(
      [&] { nc::json_to_parameters(json::parse(R"({"zero_block": [1]})"), s, bad, registry, {.path = "m"}); })};
  EXPECT_NE(msg.find("m.zero_block: zero_block must be"), std::string::npos) << msg;
}
