#include "numsim-core/input_parameter_controller.h"
#include <any>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

// Mock implementation of ParameterHandler class.
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

// Example program demonstrating usage of input_parameter_controller.
int main() {
  try {
    // Create the parameter handler and controller.
    MockParameterHandler handler;
    numsim_core::input_parameter_controller<std::string, MockParameterHandler>
        controller;

    // Insert a required parameter.
    auto &requiredParam = controller.insert<int>("required_param");
    requiredParam.add<numsim_core::is_required>();

    // Insert a parameter with a range check.
    auto &rangeParam = controller.insert<int>("range_param");
    rangeParam.add<numsim_core::check_range>(0, 100);

    // Insert a parameter with a default value.
    auto &defaultParam = controller.insert<int>("default_param");
    defaultParam.add<numsim_core::set_default>(42);

    // Set values in the handler.
    handler.insert("required_param", 10); // Required parameter is provided.
    handler.insert("range_param", 50);    // Within range.

    // Validate parameters.
    controller.check_parameter(handler);

    // Display parameter values.
    std::cout << "required_param: " << handler.get<int>("required_param")
              << std::endl;
    std::cout << "range_param: " << handler.get<int>("range_param")
              << std::endl;
    std::cout << "default_param: " << handler.get<int>("default_param")
              << std::endl;
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
