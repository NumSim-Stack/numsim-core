#ifndef NUMSIM_CORE_JSON_JSON_PARAMETER_CONVERTER_H
#define NUMSIM_CORE_JSON_JSON_PARAMETER_CONVERTER_H

#include <numsim-core/input_parameter_controller.h>
#include <numsim-core/print.h>

#include <any>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/// JSON object + parameter schema -> validated parameter_handler.
///
/// Generic over the JSON library: json_adapter<JsonType> describes the few
/// operations needed; the default works with nlohmann::json. numsim-core
/// itself does not depend on a JSON library, only the code that
/// instantiates these templates does.
namespace numsim::core {

// --- JSON adapter traits ---

template <typename JsonType> struct json_adapter {
  static bool contains(const JsonType &j, const std::string &key) { return j.contains(key); }
  static const JsonType &at(const JsonType &j, const std::string &key) { return j.at(key); }
  static bool is_object(const JsonType &j) { return j.is_object(); }
  static bool is_array(const JsonType &j) { return j.is_array(); }
  static bool is_string(const JsonType &j) { return j.is_string(); }
  static bool is_number(const JsonType &j) { return j.is_number(); }
  template <typename T> static T get(const JsonType &j) { return j.template get<T>(); }

  template <typename Fn> static void for_each_key(const JsonType &j, Fn &&fn) {
    for (auto it = j.begin(); it != j.end(); ++it)
      fn(it.key());
  }
};

// --- Reader registry ---
// Maps a parameter's C++ type to a function reading it from a JSON value.

template <typename JsonType> class json_reader_registry {
public:
  using reader_fn = std::function<std::any(const JsonType &)>;
  using adapter = json_adapter<JsonType>;

  /// Register a type read with the JSON library's own conversion.
  template <typename T> json_reader_registry &add() {
    m_readers[typeid(T)] = [](const JsonType &j) -> std::any { return adapter::template get<T>(j); };
    return *this;
  }

  /// Register a type with a custom reader.
  template <typename T> json_reader_registry &add(reader_fn fn) {
    m_readers[typeid(T)] = std::move(fn);
    return *this;
  }

  /// Read a value; `where` names the parameter in error messages.
  std::any read(std::type_index tid, const JsonType &j, const std::string &where) const {
    auto it = m_readers.find(tid);
    if (it == m_readers.end())
      throw std::invalid_argument(where + ": no JSON reader for the parameter's type");
    try {
      return it->second(j);
    } catch (const std::exception &e) {
      throw std::invalid_argument(where + ": " + e.what());
    }
  }

private:
  std::unordered_map<std::type_index, reader_fn> m_readers;
};

/// Readers for the generic parameter types; libraries add their own types.
template <typename JsonType> json_reader_registry<JsonType> make_default_json_registry() {
  using adapter = json_adapter<JsonType>;
  json_reader_registry<JsonType> reg;
  reg.template add<double>();
  reg.template add<float>();
  reg.template add<int>();
  reg.template add<std::size_t>();
  reg.template add<bool>();
  reg.template add<std::string>();
  reg.template add<std::vector<double>>();
  reg.template add<std::vector<int>>();
  reg.template add<std::vector<std::string>>();
  reg.template add<std::vector<std::size_t>>([](const JsonType &j) -> std::any {
    std::vector<std::size_t> result;
    for (const auto &elem : j)
      result.push_back(adapter::template get<std::size_t>(elem));
    return result;
  });
  reg.template add<std::unordered_map<std::string, std::vector<std::string>>>();
  return reg;
}

// --- Visitor ---
// Implements parameter_visitor_base: the schema asks for values by key.

template <typename JsonType, typename KeyType = std::string>
class json_parameter_visitor final : public parameter_visitor_base<KeyType> {
public:
  using adapter = json_adapter<JsonType>;
  using registry_type = json_reader_registry<JsonType>;

  json_parameter_visitor(const JsonType &json, const registry_type &registry, std::string path = {})
      : m_json(json), m_registry(registry), m_path(std::move(path)) {}

  bool contains(const KeyType &key) const override { return adapter::contains(m_json, key); }

  std::any read(const KeyType &key, std::type_index tid) const override {
    return m_registry.read(tid, adapter::at(m_json, key), qualified(key));
  }

  std::string qualified(const KeyType &key) const {
    return m_path.empty() ? std::string{key} : m_path + "." + std::string{key};
  }

private:
  const JsonType &m_json;
  const registry_type &m_registry;
  std::string m_path;
};

// --- Conversion ---

/// What to do with JSON keys the schema does not declare (usually typos).
enum class unknown_key_policy { warn, error, ignore };

struct json_conversion_options {
  unknown_key_policy unknown_keys{unknown_key_policy::warn};
  /// Location of the object in the document ("objects[3]", "rve.solver"),
  /// prefixed to every error message.
  std::string path{};
  /// Keys of the document format itself, never reported as unknown.
  std::vector<std::string> reserved_keys{"type", "name"};
};

/// Convert a JSON object into a validated parameter_handler: every
/// parameter of the schema is read (reader registry), defaults applied and
/// checks run. Errors are std::invalid_argument naming the path of the
/// offending parameter.
template <typename JsonType, typename KeyType, typename ParameterHandler>
void json_to_parameters(const JsonType &json,
                        const input_parameter_controller<KeyType, ParameterHandler> &schema,
                        ParameterHandler &params, const json_reader_registry<JsonType> &registry,
                        const json_conversion_options &options = {}) {
  using adapter = json_adapter<JsonType>;
  json_parameter_visitor<JsonType, KeyType> visitor(json, registry, options.path);

  if (!adapter::is_object(json))
    throw std::invalid_argument((options.path.empty() ? std::string{"parameters"} : options.path) +
                                ": expected a JSON object");

  if (options.unknown_keys != unknown_key_policy::ignore) {
    std::unordered_set<std::string> known(options.reserved_keys.begin(), options.reserved_keys.end());
    for (const auto &[key, _] : schema)
      known.insert(std::string{key});
    std::vector<std::string> unknown;
    adapter::for_each_key(json, [&](const std::string &key) {
      if (!known.contains(key))
        unknown.push_back(key);
    });
    for (const auto &key : unknown) {
      if (options.unknown_keys == unknown_key_policy::error)
        throw std::invalid_argument(visitor.qualified(key) + ": unknown parameter");
      println(stderr, "  warning: {}: unknown parameter (not in the schema)", visitor.qualified(key));
    }
  }

  try {
    schema.accept(visitor, params);
  } catch (const std::invalid_argument &e) {
    // reader errors already carry the full path; validation errors do not
    std::string const what{e.what()};
    if (!options.path.empty() && what.rfind(options.path, 0) != 0)
      throw std::invalid_argument(options.path + ": " + what);
    throw;
  }
}

/// json_to_parameters with the default reader registry.
template <typename JsonType, typename KeyType, typename ParameterHandler>
void json_to_parameters(const JsonType &json,
                        const input_parameter_controller<KeyType, ParameterHandler> &schema,
                        ParameterHandler &params, const json_conversion_options &options = {}) {
  static const auto registry{make_default_json_registry<JsonType>()};
  json_to_parameters(json, schema, params, registry, options);
}

} // namespace numsim::core

#endif // NUMSIM_CORE_JSON_JSON_PARAMETER_CONVERTER_H
