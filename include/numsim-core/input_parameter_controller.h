#ifndef INPUT_PARAMETER_CONTROLLER_H
#define INPUT_PARAMETER_CONTROLLER_H

#include <any>
#include <exception>
#include <fstream>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "input_parameter_enum_utils.h"

namespace numsim_core {

/**
 * @file input_parameter_controller.h
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
  input_parameter_check_base const &operator=(input_parameter_check_base &&) = delete;

  /**
   * @brief Deleted copy assignment operator.
   */
  input_parameter_check_base const &operator=(input_parameter_check_base const &) = delete;

  /**
   * @brief Pure virtual function to check the parameter.
   *
   * @param input The parameter handler to check against.
   */
  virtual void check(ParameterHandler &) const = 0;

  void set_description(std::string desc) { m_description = std::move(desc); }

  auto const& description() const { return m_description; }

protected:
  /**
   * @brief Reference to the input_parameter.
   */
  input_parameter<T, KeyType, ParameterHandler> const &m_para;

  std::string m_description;
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
                                    " = " + std::to_string(value) +
                                    " is out of range [" + std::to_string(m_low) +
                                    ", " + std::to_string(m_high) + "]");
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
 * If the parameter is missing from the handler, inserts the default value.
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
                       T &&value = T()) noexcept
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
 * @brief Class for validating the data type of an input parameter.
 *
 * Inherits from input_parameter_check_base and implements the check method.
 * Attempts to cast the stored value to the expected type T.
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
   * @brief Constructs check_data_type with a reference to an input_parameter.
   *
   * @param para The input_parameter object to check.
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
   * @brief Checks if the stored parameter value can be cast to type T.
   *
   * @param input The parameter handler to check against.
   */
  void check(ParameterHandler &input) const final override {
    if (input.contains(this->m_para.name())) {
      try {
        [[maybe_unused]] const auto value{input.template get<T>(this->m_para.name())};
      } catch (const std::exception& e) {
        throw std::invalid_argument(
            "Parameter '" + this->m_para.name() + "' has wrong type: " + e.what());
      }
    }
  }
};

/**
 * @brief Class for validating an enum parameter from its string representation.
 *
 * Validates that the string value is a valid enum entry using enum_traits,
 * then replaces the string in the handler with the converted enum value.
 *
 * @tparam EnumType The enum type to validate against.
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename EnumType, typename KeyType, typename ParameterHandler>
class check_enum final
    : public input_parameter_check_base<EnumType, KeyType, ParameterHandler> {
public:
  using base = input_parameter_check_base<EnumType, KeyType, ParameterHandler>;

  explicit check_enum(input_parameter<EnumType, KeyType, ParameterHandler> const& para) noexcept
      : base(para) {}

  void check(ParameterHandler &input) const final override {
    const auto &name = this->m_para.name();

    if (!input.contains(name))
      throw std::invalid_argument("Enum parameter '" + name + "' is missing!");

    const auto& str_val = input.template get<std::string>(name);

    if (!enum_traits<EnumType>::is_valid(str_val)) {
      throw std::invalid_argument("Invalid enum value for '" + name + "': '" + str_val + "'");
    }

    input.insert(name, enum_traits<EnumType>::from_string(str_val));
  }
};

// --- parameter_visitor_base ---

/**
 * @brief Abstract base for visiting input parameters.
 *
 * A pure data reader: reads values from an external source (JSON, YAML, etc.)
 * and returns them as std::any. Does NOT know about ParameterHandler — the
 * typed insertion is handled by input_parameter<T>::accept().
 *
 * Concrete visitors inherit from this and implement contains() + read().
 *
 * @tparam KeyType The type used as the key for the parameters.
 */
template <typename KeyType>
class parameter_visitor_base {
public:
  virtual ~parameter_visitor_base() = default;

  /**
   * @brief Check if the data source contains a value for the given key.
   *
   * @param key The parameter name.
   * @return True if the data source has a value for this key.
   */
  virtual bool contains(const KeyType& key) const = 0;

  /**
   * @brief Read a value from the data source.
   *
   * Called by input_parameter<T>::accept(). The implementation uses tid
   * to determine the expected C++ type and returns the converted value
   * wrapped in std::any.
   *
   * @param key The parameter name.
   * @param tid The runtime type identifier (typeid(T) from the schema).
   * @return The converted value as std::any.
   * @throws std::runtime_error if the type is not supported or conversion fails.
   */
  virtual std::any read(const KeyType& key, std::type_index tid) const = 0;
};

// --- input_parameter_base ---

/**
 * @brief Base class for input parameters.
 *
 * Provides the basic structure for an input parameter, including its name,
 * runtime type identification, child parameters, and description.
 *
 * @tparam KeyType The type used as the key for the parameter.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename KeyType, typename ParameterHandler>
class input_parameter_base {
public:
  using child_type = std::unique_ptr<input_parameter_base<KeyType, ParameterHandler>>;

  /**
   * @brief Constructs input_parameter_base with a name.
   *
   * @param name The name of the parameter.
   */
  explicit input_parameter_base(const KeyType &name) noexcept
      : m_name(name) {}

  /**
   * @brief Constructs input_parameter_base with an rvalue name.
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
   * @brief Returns the runtime type identifier for this parameter's C++ type.
   *
   * Used by external converters (e.g., JSON) to determine the expected type
   * without requiring a compile-time variant of all possible types.
   *
   * @return std::type_index identifying the parameter's type T.
   */
  virtual std::type_index type_id() const = 0;

  /**
   * @brief Accept a visitor for this parameter.
   *
   * The visitor reads the value from its data source and returns std::any.
   * The concrete input_parameter<T> does the typed insertion into params
   * via std::any_cast<T>.
   *
   * @param visitor The visitor (data reader) to accept.
   * @param params The parameter handler to insert the converted value into.
   */
  virtual void accept(const parameter_visitor_base<KeyType>& visitor,
                      ParameterHandler& params) const = 0;

  /**
   * @brief Returns the name of the parameter.
   *
   * @return The name of the parameter.
   */
  inline const auto &name() const { return m_name; }

  /**
   * @brief Adds a typed child parameter.
   *
   * @tparam T The type of the child parameter.
   * @param name The name of the child parameter.
   * @return Reference to the newly created child input_parameter.
   */
  template <typename T>
  auto &add_child(KeyType const &name) {
    m_child.push_back(std::make_unique<input_parameter<T, KeyType, ParameterHandler>>(name));
    return static_cast<input_parameter<T, KeyType, ParameterHandler>&>(*m_child.back());
  }

  /**
   * @brief Returns the list of child parameters.
   *
   * @return Const reference to the child parameter list.
   */
  inline auto const& childs() const { return m_child; }

  /**
   * @brief Sets the description for this parameter.
   *
   * @param desc The description string.
   * @return Reference to this object for chaining.
   */
  inline auto& description(std::string && desc) { m_description = std::move(desc); return *this; }

  /**
   * @brief Returns the description of the parameter.
   *
   * @return Const reference to the description string.
   */
  inline std::string const& description() const { return m_description; }

  /**
   * @brief Sets the minimum-value hint for this parameter.
   *
   * Stored as double regardless of the parameter's underlying C++ type;
   * downcasting is the consumer's responsibility. Used by GUIs (e.g. Tessera)
   * to bound spinbox ranges; not enforced by check_parameter (use
   * `.add<check_range>(low, high)` for runtime validation).
   *
   * @param v The minimum value hint.
   * @return Reference to this object for chaining.
   */
  inline auto& min(double v) noexcept { m_min = v; return *this; }

  /**
   * @brief Sets the maximum-value hint for this parameter.
   *
   * @param v The maximum value hint.
   * @return Reference to this object for chaining.
   */
  inline auto& max(double v) noexcept { m_max = v; return *this; }

  /**
   * @brief Sets the units string for this parameter (e.g. "m", "rad", "%").
   *
   * Used by GUIs to suffix labels/spinboxes; ignored by validation.
   *
   * @param u The units string.
   * @return Reference to this object for chaining.
   */
  inline auto& units(std::string u) { m_units = std::move(u); return *this; }

  /**
   * @brief Returns the minimum-value hint, if set.
   */
  inline std::optional<double> const& min() const noexcept { return m_min; }

  /**
   * @brief Returns the maximum-value hint, if set.
   */
  inline std::optional<double> const& max() const noexcept { return m_max; }

  /**
   * @brief Returns the units string (empty if unset).
   */
  inline std::string const& units() const noexcept { return m_units; }

protected:
  const KeyType m_name; ///< Name of the parameter.
  std::list<child_type> m_child; ///< Child parameters.
  std::string m_description; ///< Parameter description.
  std::optional<double> m_min; ///< Minimum-value hint for GUIs.
  std::optional<double> m_max; ///< Maximum-value hint for GUIs.
  std::string m_units; ///< Units string for GUIs (e.g. "m", "rad").
};

// --- input_parameter<T> ---

/**
 * @brief Typed input parameter with validation checks.
 *
 * Stores the concrete type T as a template parameter, provides runtime
 * type identification via type_id(), and manages a list of validation checks.
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
   * @brief Constructs input_parameter with an rvalue name.
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
   * @brief Returns the runtime type identifier for type T.
   *
   * @return std::type_index for typeid(T).
   */
  std::type_index type_id() const override { return typeid(T); }

  /**
   * @brief Accept a visitor: read value via visitor, insert with correct type.
   *
   * This is where the dispatch happens. The visitor reads from its data source
   * and returns std::any. This method knows T, so it does the typed insertion
   * into the parameter handler via std::any_cast<T>.
   */
  void accept(const parameter_visitor_base<KeyType>& visitor,
              ParameterHandler& params) const override {
    if (!visitor.contains(this->m_name)) return;
    auto value = visitor.read(this->m_name, typeid(T));
    try {
      params.template insert<T>(this->m_name, std::any_cast<T>(std::move(value)));
    } catch (const std::bad_any_cast&) {
      throw std::runtime_error(
          "input_parameter::accept(): type mismatch for '" + this->m_name +
          "' — reader returned wrong type");
    }
  }

  /**
   * @brief Adds a validation check for the parameter.
   *
   * @tparam Check The check class template (e.g., is_required, set_default).
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

// --- input_parameter_controller ---

/**
 * @brief Controller for managing a collection of input parameters.
 *
 * Manages a map of named input parameters with their validation rules.
 * Each parameter's expected C++ type is tracked via type_id() on the
 * type-erased input_parameter_base, enabling runtime type dispatch
 * (e.g., for JSON conversion) without requiring a compile-time variant.
 *
 * @tparam KeyType The type used as the key for the parameters.
 * @tparam ParameterHandler The type of the handler that manages parameters.
 */
template <typename KeyType, typename ParameterHandler>
class input_parameter_controller {
public:
  using key_type = KeyType;
  using parameter_handler = ParameterHandler;

  /**
   * @brief Default constructor.
   */
  input_parameter_controller() = default;

  /**
   * @brief Deleted copy constructor.
   */
  input_parameter_controller(input_parameter_controller const &) = delete;

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
  input_parameter_controller const &operator=(input_parameter_controller const &) = delete;

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
    para = std::make_unique<input_parameter<T, KeyType, ParameterHandler>>(name);
    return *static_cast<input_parameter<T, KeyType, ParameterHandler> *>(para.get());
  }

  /**
   * @brief Retrieves a parameter by its name.
   *
   * @param name The name of the parameter to retrieve.
   * @return Const reference to the input_parameter_base.
   */
  const input_parameter_base<KeyType, ParameterHandler> &get(KeyType const &name) const {
    return *m_data.at(name).get();
  }

  /**
   * @brief Checks all parameters in the controller against the provided handler.
   *
   * @param parameter The parameter handler to check against.
   */
  auto check_parameter(ParameterHandler &parameter) const {
    for (const auto &[key, check] : m_data) {
      check->check_parameter(parameter);
    }
  }

  /**
   * @brief Accept a visitor for all parameters: read, insert, then validate.
   *
   * Pairs {schema, data_source} → validated parameter_handler:
   * 1. Each input_parameter<T> reads from the visitor and inserts into params.
   * 2. Defaults are applied for missing parameters (with logging).
   * 3. Remaining missing required parameters are collected and reported as one error.
   * 4. Other validation checks (range, type) run and throw immediately on failure.
   *
   * @param visitor The data source visitor.
   * @param params The parameter handler to fill and validate.
   * @throws std::invalid_argument if any required parameters are missing or validation fails.
   */
  void accept(const parameter_visitor_base<KeyType>& visitor,
              ParameterHandler& params) const {
    // 1. Read values from visitor
    std::unordered_set<KeyType> provided;
    for (const auto& [key, param_ptr] : m_data) {
      param_ptr->accept(visitor, params);
      if (params.contains(key))
        provided.insert(key);
    }

    // 2. Run checks — this applies defaults and validates
    //    Separate is_required failures from other validation errors.
    std::vector<KeyType> missing;
    for (const auto& [key, param_ptr] : m_data) {
      try {
        param_ptr->check_parameter(params);
      } catch (const std::invalid_argument&) {
        // Only collect as "missing" if params still doesn't have it
        // (set_default would have inserted it, so this is truly missing)
        if (!params.contains(key))
          missing.push_back(key);
        else
          throw; // re-throw range/type errors (value exists but is invalid)
      }
    }

    // 3. Log defaults that were applied
    for (const auto& [key, param_ptr] : m_data) {
      if (!provided.contains(key) && params.contains(key))
        std::println("    using default: '{}'", key);
    }

    // 4. Report all missing required parameters at once
    if (!missing.empty()) {
      std::println("  missing required parameters:");
      for (const auto& key : missing)
        std::println("    - {}", key);
      throw std::invalid_argument(
          "missing " + std::to_string(missing.size()) + " required parameter(s)");
    }
  }

  auto contains(KeyType const &name) { return m_data.contains(name); }

  auto begin() { return m_data.begin(); }
  auto end() { return m_data.end(); }
  auto begin() const { return m_data.begin(); }
  auto end() const { return m_data.end(); }

private:
  std::unordered_map<KeyType,
                     std::unique_ptr<input_parameter_base<KeyType, ParameterHandler>>>
      m_data; ///< Map of parameters managed by the controller.
};

} // namespace numsim_core
#endif // INPUT_PARAMETER_CONTROLLER_H
