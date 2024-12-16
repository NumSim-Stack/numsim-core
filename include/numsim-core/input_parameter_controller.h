#ifndef INPUT_PARAMETER_CONTROLLER_H
#define INPUT_PARAMETER_CONTROLLER_H

#include <any>
#include <exception>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace numsim_core {

/**
 * @file InputParameter.h
 * @brief Contains definitions for input parameter handling and validation.
 */

/**
 * @brief Forward declaration of the input_parameter class template.
 *
 * @tparam T The type of the parameter.
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename T, typename KeyType, typename ParameterHandler>
class input_parameter;

/**
 * @brief Base class for input parameter checks.
 *
 * This class is abstract and defines a common interface for parameter validation.
 *
 * @tparam T The type of the parameter.
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename T, typename KeyType, typename ParameterHandler>
class input_parameter_check_base {
public:
  /**
   * @brief Deleted default constructor.
   */
  input_parameter_check_base() = delete;

  /**
   * @brief Deleted copy constructor.
   */
  input_parameter_check_base(input_parameter_check_base const &) = delete;

  /**
   * @brief Deleted move constructor.
   */
  input_parameter_check_base(input_parameter_check_base &&) = delete;

  /**
   * @brief Constructs the input_parameter_check_base with a reference to an input_parameter.
   *
   * @param para The input_parameter object to be referenced.
   */
  explicit input_parameter_check_base(
      input_parameter<T, KeyType, ParameterHandler> const &para) noexcept
      : m_para(para) {}

  /**
   * @brief Virtual destructor.
   */
  virtual ~input_parameter_check_base() = default;

  /**
   * @brief Deleted move assignment operator.
   */
  input_parameter_check_base const &
  operator=(input_parameter_check_base &&) = delete;

  /**
   * @brief Deleted copy assignment operator.
   */
  input_parameter_check_base const &
  operator=(input_parameter_check_base const &) = delete;

  /**
   * @brief Pure virtual function to check the parameter.
   *
   * @param input The parameter handler to check against.
   */
  virtual void check(ParameterHandler &) const = 0;

protected:
  /**
   * @brief Reference to the input_parameter.
   */
  input_parameter<T, KeyType, ParameterHandler> const &m_para;
};

/**
 * @brief Class for checking if an input parameter is required.
 *
 * Inherits from input_parameter_check_base and implements the check method.
 *
 * @tparam T The type of the parameter.
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename T, typename KeyType, typename ParameterHandler>
class is_required final
    : public input_parameter_check_base<T, KeyType, ParameterHandler> {
public:
  /**
   * @brief Alias for the base class.
   */
  using base = input_parameter_check_base<T, KeyType, ParameterHandler>;

  /**
   * @brief Deleted default constructor.
   */
  is_required() = delete;

  /**
   * @brief Deleted copy constructor.
   */
  is_required(is_required const &) = delete;

  /**
   * @brief Deleted move constructor.
   */
  is_required(is_required &&) = delete;

  /**
   * @brief Constructs is_required with a reference to an input_parameter.
   *
   * @param para The input_parameter object to check.
   */
  explicit is_required(
      input_parameter<T, KeyType, ParameterHandler> const &para) noexcept
      : base(para) {}

  /**
   * @brief Deleted copy assignment operator.
   */
  is_required const &operator=(is_required const &) = delete;

  /**
   * @brief Deleted move assignment operator.
   */
  is_required const &operator=(is_required &&) = delete;

  /**
   * @brief Checks if the parameter is present in the handler.
   *
   * Throws an invalid_argument exception if the parameter is missing.
   *
   * @param input The parameter handler to check against.
   */
  void check(ParameterHandler &input) const final override {
    if (!input.contains(this->m_para.name())) {
      throw std::invalid_argument("Parameter " + this->m_para.name() +
                                  " is missing!");
    }
  }
};

/**
 * @brief Class for checking if an input parameter is within a specific range.
 *
 * Inherits from input_parameter_check_base and implements the check method.
 *
 * @tparam T The type of the parameter.
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename T, typename KeyType, typename ParameterHandler>
class check_range final
    : public input_parameter_check_base<T, KeyType, ParameterHandler> {
public:
  /**
   * @brief Alias for the base class.
   */
  using base = input_parameter_check_base<T, KeyType, ParameterHandler>;

  /**
   * @brief Deleted default constructor.
   */
  check_range() = delete;

  /**
   * @brief Deleted copy constructor.
   */
  check_range(check_range const &) = delete;

  /**
   * @brief Deleted move constructor.
   */
  check_range(check_range &&) = delete;

  /**
   * @brief Constructs check_range with a reference to an input_parameter and range bounds.
   *
   * @param para The input_parameter object to check.
   * @param lower The lower bound of the range.
   * @param higher The upper bound of the range.
   */
  explicit check_range(input_parameter<T, KeyType, ParameterHandler> const &para,
                       T &&lower, T &&higher) noexcept
      : base(para), m_low(std::forward<T>(lower)),
        m_high(std::forward<T>(higher)) {}

  /**
   * @brief Deleted copy assignment operator.
   */
  check_range const &operator=(check_range const &) = delete;

  /**
   * @brief Deleted move assignment operator.
   */
  check_range const &operator=(check_range &&) = delete;

  /**
   * @brief Checks if the parameter is within the specified range.
   *
   * Throws an invalid_argument exception if the value is outside the defined range.
   *
   * @param input The parameter handler to check against.
   */
  void check(ParameterHandler &input) const final override {
    const auto &para_name{this->m_para.name()};
    if (input.contains(para_name)) {
      const auto value{input.template get<T>(para_name)};
      if (value < m_low || value > m_high) {
        throw std::invalid_argument("Parameter " + this->m_para.name() +
                                    " out of range");
      }
    }
  }

private:
  const T m_low;  ///< Lower bound of the range.
  const T m_high; ///< Upper bound of the range.
};

/**
 * @brief Class for setting a default value for an input parameter.
 *
 * Inherits from input_parameter_check_base and implements the check method.
 *
 * @tparam T The type of the parameter.
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename T, typename KeyType, typename ParameterHandler>
class set_default final
    : public input_parameter_check_base<T, KeyType, ParameterHandler> {
public:
  /**
   * @brief Alias for the base class.
   */
  using base = input_parameter_check_base<T, KeyType, ParameterHandler>;

  /**
   * @brief Deleted default constructor.
   */
  set_default() = delete;

  /**
   * @brief Deleted copy constructor.
   */
  set_default(set_default const &) = delete;

  /**
   * @brief Deleted move constructor.
   */
  set_default(set_default &&) = delete;

  /**
   * @brief Constructs set_default with a reference to an input_parameter and a default value.
   *
   * @param para The input_parameter object to set a default for.
   * @param value The default value to set.
   */
  explicit set_default(input_parameter<T, KeyType, ParameterHandler> const &para,
                       T &&value) noexcept
      : base(para), m_value(value) {}

  /**
   * @brief Deleted copy assignment operator.
   */
  set_default const &operator=(set_default const &) = delete;

  /**
   * @brief Deleted move assignment operator.
   */
  set_default const &operator=(set_default &&) = delete;

  /**
   * @brief Checks if the parameter is present in the handler and sets default if absent.
   *
   * @param input The parameter handler to check against.
   */
  void check(ParameterHandler &input) const final override {
    if (!input.contains(this->m_para.name())) {
      input.insert(this->m_para.name(), m_value);
    }
  }

private:
  const T m_value; ///< Default value for the parameter.
};

/**
 * @brief Class for setting a default value for an input parameter.
 *
 * Inherits from input_parameter_check_base and implements the check method.
 *
 * @tparam T The type of the parameter.
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename T, typename KeyType, typename ParameterHandler>
class check_data_type final
    : public input_parameter_check_base<T, KeyType, ParameterHandler> {
public:
  /**
   * @brief Alias for the base class.
   */
  using base = input_parameter_check_base<T, KeyType, ParameterHandler>;

  /**
   * @brief Deleted default constructor.
   */
  check_data_type() = delete;

  /**
   * @brief Deleted copy constructor.
   */
  check_data_type(check_data_type const &) = delete;

  /**
   * @brief Deleted move constructor.
   */
  check_data_type(check_data_type &&) = delete;

  /**
   * @brief Constructs set_default with a reference to an input_parameter and a default value.
   *
   * @param para The input_parameter object to set a default for.
   * @param value The default value to set.
   */
  explicit check_data_type(input_parameter<T, KeyType, ParameterHandler> const &para) noexcept
      : base(para) {}

  /**
   * @brief Deleted copy assignment operator.
   */
  check_data_type const &operator=(check_data_type const &) = delete;

  /**
   * @brief Deleted move assignment operator.
   */
  check_data_type const &operator=(check_data_type &&) = delete;

  /**
   * @brief Checks if the parameter is present in the handler and sets default if absent.
   *
   * @param input The parameter handler to check against.
   */
  void check(ParameterHandler &input) const final override {
    if (input.contains(this->m_para.name())) {
      try {
        const auto value{input.template get<T>(this->m_para.name())};
      } catch (...) {
        throw;
      }
    }
  }
};

/**
 * @brief Base class for input parameters.
 *
 * This class provides the basic structure for an input parameter, including its name.
 *
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename KeyType, typename ParameterHandler>
class input_parameter_base {
public:
  /**
   * @brief Constructs input_parameter_base with a name.
   *
   * @param name The name of the parameter.
   */
  explicit input_parameter_base(const KeyType &name) noexcept
      : m_name(name) {}

  /**
   * @brief Constructs input_parameter_base with a rvalue name.
   *
   * @param name The name of the parameter.
   */
  explicit input_parameter_base(KeyType &&name) noexcept
      : m_name(std::forward<KeyType>(name)) {}

  /**
   * @brief Virtual destructor.
   */
  virtual ~input_parameter_base() = default;

  /**
   * @brief Deleted copy constructor.
   */
  input_parameter_base(input_parameter_base const &) = delete;

  /**
   * @brief Deleted move constructor.
   */
  input_parameter_base(input_parameter_base &&) = delete;

  /**
   * @brief Deleted copy assignment operator.
   */
  input_parameter_base const &operator=(input_parameter_base const &) = delete;

  /**
   * @brief Deleted move assignment operator.
   */
  input_parameter_base const &operator=(input_parameter_base &&) = delete;

  /**
   * @brief Pure virtual function to check the parameter.
   *
   * @param input The parameter handler to check against.
   */
  virtual void check_parameter(ParameterHandler &) const = 0;

  /**
   * @brief Returns the name of the parameter.
   *
   * @return The name of the parameter.
   */
  inline const auto &name() const { return m_name; }

private:
  const KeyType m_name; ///< Name of the parameter.
};

/**
 * @brief Class for managing input parameters.
 *
 * This class allows the registration and validation of input parameters.
 *
 * @tparam T The type of the parameter.
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename T, typename KeyType, typename ParameterHandler>
class input_parameter final : public input_parameter_base<KeyType, ParameterHandler> {
public:
  /**
   * @brief Alias for the base class.
   */
  using base = input_parameter_base<KeyType, ParameterHandler>;

  /**
   * @brief Constructs input_parameter with a name.
   *
   * @param name The name of the parameter.
   */
  explicit input_parameter(const KeyType &name) noexcept : base(name) {}

  /**
   * @brief Constructs input_parameter with a rvalue name.
   *
   * @param name The name of the parameter.
   */
  explicit input_parameter(KeyType &&name) noexcept
      : base(std::forward<KeyType>(name)) {}

  /**
   * @brief Deleted default constructor.
   */
  input_parameter() = delete;

  /**
   * @brief Destructor.
   */
  ~input_parameter() = default;

  /**
   * @brief Checks all registered validations for the parameter.
   *
   * @param input The parameter handler to check against.
   */
  void check_parameter(ParameterHandler &input) const override {
    for (auto &check : m_checks) {
      check->check(input);
    }
  }

  /**
   * @brief Adds a validation check for the parameter.
   *
   * @tparam Check The type of the check class.
   * @tparam Args The arguments to pass to the check constructor.
   * @param args The arguments to pass to the check constructor.
   * @return Reference to this input_parameter for chaining.
   */
  template <template <class _T, class _KeyType, class _ParameterHandler> class Check,
            typename... Args>
  auto &add(Args &&...args) {
    m_checks.push_back(std::make_unique<Check<T, KeyType, ParameterHandler>>(
        *this, std::forward<Args>(args)...));
    return *this;
  }

private:
  std::vector<std::unique_ptr<input_parameter_check_base<T, KeyType, ParameterHandler>>>
      m_checks; ///< List of checks associated with the input parameter.
};

/**
 * @brief Class for controlling input parameters.
 *
 * This class manages a collection of input parameters and their validations.
 *
 * @tparam KeyType The type used as the key for the parameters.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename KeyType, typename ParameterHandler>
class input_parameter_controller {
public:
  /**
   * @brief Default constructor.
   */
  input_parameter_controller() = default;

  /**
   * @brief Deleted copy constructor.
   */
  input_parameter_controller(input_parameter_controller const &data) = delete;

  /**
   * @brief Move constructor.
   *
   * @param data The input_parameter_controller to move from.
   */
  input_parameter_controller(input_parameter_controller &&data)
      : m_data(std::move(data.m_data)) {}

  /**
   * @brief Move assignment operator.
   *
   * @param data The input_parameter_controller to move from.
   * @return Reference to this object.
   */
  input_parameter_controller const &operator=(input_parameter_controller &&data) {
    if (m_data.empty()) {
      m_data = std::move(data.m_data);
      return *this;
    }
    for (auto &[key, check] : data.m_data) {
      m_data[key] = std::move(check);
    }
    return *this;
  }

  /**
   * @brief Deleted copy assignment operator.
   */
  input_parameter_controller const &
  operator=(input_parameter_controller const &data) = delete;

  /**
   * @brief Inserts a new input parameter into the controller.
   *
   * @tparam T The type of the parameter.
   * @param name The name of the parameter to insert.
   * @return Reference to the newly created input_parameter.
   */
  template <typename T>
  input_parameter<T, KeyType, ParameterHandler> &insert(KeyType const &name) {
    auto &para{m_data[name]};
    para =
        std::move(std::make_unique<input_parameter<T, KeyType, ParameterHandler>>(name));
    return *static_cast<input_parameter<T, KeyType, ParameterHandler> *>(para.get());
  }

  /**
   * @brief Retrieves a parameter by its name.
   *
   * @param name The name of the parameter to retrieve.
   * @return Reference to the input_parameter_base.
   */
  const input_parameter_base<KeyType, ParameterHandler> &get(KeyType const &name) {
    return *m_data[name].get();
  }

  /**
   * @brief Checks all parameters in the controller against the provided handler.
   *
   * @param parameter The parameter handler to check against.
   */
  auto check_parameter(ParameterHandler &parameter) {
    for (const auto &[key, check] : m_data) {
      check->check_parameter(parameter);
    }
  }

private:
  std::unordered_map<KeyType,
                     std::unique_ptr<input_parameter_base<KeyType, ParameterHandler>>>
      m_data; ///< Map of parameters managed by the controller.
};

} // namespace uvwCommon
#endif // INPUT_PARAMETER_CONTROLLER_H
