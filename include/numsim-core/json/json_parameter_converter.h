#ifndef NUMSIM_CORE_JSON_JSON_PARAMETER_CONVERTER_H
#define NUMSIM_CORE_JSON_JSON_PARAMETER_CONVERTER_H

#include <numsim-core/input_parameter_controller.h>
#include <numsim-core/print.h>

#include <any>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/// JSON object + parameter schema -> validated parameter_handler.
///
/// numsim-core does not depend on a JSON library; only the code that
/// instantiates these templates does. json_adapter<JsonType> is the whole
/// interface the converter uses. The primary template fits nlohmann::json;
/// other libraries specialise it.
namespace numsim::core {

// --- JSON adapter traits ---

template <typename JsonType> struct json_adapter {
  static bool contains(const JsonType &j, const std::string &key) { return j.contains(key); }
  static const JsonType &at(const JsonType &j, const std::string &key) { return j.at(key); }
  static bool is_object(const JsonType &j) { return j.is_object(); }
  static bool is_array(const JsonType &j) { return j.is_array(); }
  static bool is_number(const JsonType &j) { return j.is_number(); }
  static bool is_number_integer(const JsonType &j) { return j.is_number_integer(); }
  static bool is_number_unsigned(const JsonType &j) { return j.is_number_unsigned(); }
  static std::size_t size(const JsonType &j) { return j.size(); }
  static const JsonType &element(const JsonType &j, std::size_t i) { return j.at(i); }
  /// Kind of the value for error messages ("null", "string", ...).
  static std::string type_name(const JsonType &j) { return j.type_name(); }
  /// The value as JSON text, for error messages.
  static std::string dump(const JsonType &j) { return j.dump(); }
  /// Conversion by the JSON library itself, used for types without a
  /// checked reader below (bool, string, maps, user types).
  template <typename T> static T get(const JsonType &j) { return j.template get<T>(); }

  template <typename Fn> static void for_each_key(const JsonType &j, Fn &&fn) {
    for (auto it = j.begin(); it != j.end(); ++it)
      fn(it.key());
  }
};

// --- Errors ---

/// An input error located in the document. path() is where it happened
/// ("materials[1].poisson"), message() what happened; what() is
/// "path: message".
class json_conversion_error : public std::invalid_argument {
public:
  json_conversion_error(std::string path, const std::string &message)
      : std::invalid_argument(path.empty() ? message : path + ": " + message), m_path(std::move(path)),
        m_message(message) {}

  const std::string &path() const noexcept { return m_path; }
  const std::string &message() const noexcept { return m_message; }

private:
  std::string m_path;
  std::string m_message;
};

namespace detail {

/// "a.b" + "c" -> "a.b.c", "a.b" + "[2]" -> "a.b[2]".
inline std::string join_path(const std::string &base, const std::string &rel) {
  if (rel.empty())
    return base;
  if (base.empty())
    return rel;
  return rel.front() == '[' ? base + rel : base + "." + rel;
}

template <typename T> struct is_std_vector : std::false_type {};
template <typename E, typename A> struct is_std_vector<std::vector<E, A>> : std::true_type {};

template <typename T>
concept checked_integer = std::integral<T> && !std::same_as<T, bool>;

template <typename T, typename JsonType> T read_json_value(const JsonType &j);

template <checked_integer T, typename JsonType> T read_integer(const JsonType &j) {
  using adapter = json_adapter<JsonType>;
  auto out_of_range = [&] {
    return std::invalid_argument(std::format("{} is out of range [{}, {}]", adapter::dump(j),
                                             std::numeric_limits<T>::min(), std::numeric_limits<T>::max()));
  };
  if (!adapter::is_number(j))
    throw std::invalid_argument("expected an integer, got " + adapter::type_name(j));
  if (adapter::is_number_unsigned(j)) {
    auto const v{adapter::template get<std::uintmax_t>(j)};
    if (!std::in_range<T>(v))
      throw out_of_range();
    return static_cast<T>(v);
  }
  if (adapter::is_number_integer(j)) {
    auto const v{adapter::template get<std::intmax_t>(j)};
    if (!std::in_range<T>(v))
      throw out_of_range();
    return static_cast<T>(v);
  }
  // Floating point in the document: accepted when integral (1e6, 4.0).
  auto const v{adapter::template get<double>(j)};
  if (!std::isfinite(v) || std::trunc(v) != v)
    throw std::invalid_argument("expected an integer, got " + adapter::dump(j));
  auto const lv{static_cast<long double>(v)};
  if (lv < static_cast<long double>(std::numeric_limits<T>::min()) ||
      lv > static_cast<long double>(std::numeric_limits<T>::max()))
    throw out_of_range();
  return static_cast<T>(v);
}

template <std::floating_point T, typename JsonType> T read_floating(const JsonType &j) {
  using adapter = json_adapter<JsonType>;
  if (!adapter::is_number(j))
    throw std::invalid_argument("expected a number, got " + adapter::type_name(j));
  auto const v{adapter::template get<long double>(j)};
  if (std::isfinite(v) && std::fabs(v) > static_cast<long double>(std::numeric_limits<T>::max()))
    throw std::invalid_argument(std::format("{} is out of range for {}", adapter::dump(j),
                                            sizeof(T) == sizeof(float) ? "float" : "the floating-point type"));
  return static_cast<T>(v);
}

template <typename Vector, typename JsonType> Vector read_array(const JsonType &j) {
  using adapter = json_adapter<JsonType>;
  if (!adapter::is_array(j))
    throw std::invalid_argument("expected an array, got " + adapter::type_name(j));
  Vector result;
  result.reserve(adapter::size(j));
  for (std::size_t i = 0; i < adapter::size(j); ++i) {
    auto const index{"[" + std::to_string(i) + "]"};
    try {
      result.push_back(read_json_value<typename Vector::value_type>(adapter::element(j, i)));
    } catch (const json_conversion_error &e) {
      throw json_conversion_error(join_path(index, e.path()), e.message());
    } catch (const std::exception &e) {
      throw json_conversion_error(index, e.what());
    }
  }
  return result;
}

/// Checked conversion: integers and floats must fit, lists must be arrays.
template <typename T, typename JsonType> T read_json_value(const JsonType &j) {
  if constexpr (checked_integer<T>)
    return read_integer<T>(j);
  else if constexpr (std::floating_point<T>)
    return read_floating<T>(j);
  else if constexpr (is_std_vector<T>::value)
    return read_array<T>(j);
  else
    return json_adapter<JsonType>::template get<T>(j);
}

} // namespace detail

// --- Reader registry ---
// Maps a parameter to the function reading it from a JSON value: by key
// first (add_for_key), then by C++ type (add).

template <typename JsonType> class json_reader_registry {
public:
  /// Readers get the value and the parameter's key. Readers are found by
  /// type, so the key is the only way for one to name its parameter.
  using reader_fn = std::function<std::any(const JsonType &, const std::string &)>;

  /// Register a type read by the checked default conversion.
  template <typename T> json_reader_registry &add() {
    m_readers[typeid(T)] = [](const JsonType &j, const std::string &) -> std::any {
      return detail::read_json_value<T>(j);
    };
    return *this;
  }

  /// Register a reader for a type. `fn` takes (value, key) or just (value).
  template <typename T, typename Fn> json_reader_registry &add(Fn fn) {
    m_readers[typeid(T)] = make_reader(std::move(fn));
    return *this;
  }

  /// Register a reader for one parameter; it wins over the reader for the
  /// parameter's type, so two parameters of one type can differ.
  template <typename T, typename Fn> json_reader_registry &add_for_key(std::string key, Fn fn) {
    m_keyed_readers[{std::type_index(typeid(T)), std::move(key)}] = make_reader(std::move(fn));
    return *this;
  }

  /// Read parameter `key`; errors are json_conversion_error at `where`.
  std::any read(std::type_index tid, const JsonType &j, const std::string &key, const std::string &where) const {
    const reader_fn *fn{nullptr};
    if (auto keyed = m_keyed_readers.find({tid, key}); keyed != m_keyed_readers.end())
      fn = &keyed->second;
    else if (auto it = m_readers.find(tid); it != m_readers.end())
      fn = &it->second;
    if (fn == nullptr)
      throw json_conversion_error(where, "no JSON reader for the parameter's type");
    try {
      return (*fn)(j, key);
    } catch (const json_conversion_error &e) {
      throw json_conversion_error(detail::join_path(where, e.path()), e.message());
    } catch (const std::exception &e) {
      throw json_conversion_error(where, e.what());
    }
  }

private:
  template <typename Fn> static reader_fn make_reader(Fn fn) {
    if constexpr (std::is_invocable_r_v<std::any, Fn &, const JsonType &, const std::string &>)
      return reader_fn(std::move(fn));
    else
      return [fn = std::move(fn)](const JsonType &j, const std::string &) -> std::any { return fn(j); };
  }

  struct keyed_hash {
    std::size_t operator()(const std::pair<std::type_index, std::string> &k) const noexcept {
      return std::hash<std::type_index>{}(k.first) ^ (std::hash<std::string>{}(k.second) << 1);
    }
  };

  std::unordered_map<std::type_index, reader_fn> m_readers;
  std::unordered_map<std::pair<std::type_index, std::string>, reader_fn, keyed_hash> m_keyed_readers;
};

/// Readers for the generic parameter types; libraries add their own types.
template <typename JsonType> json_reader_registry<JsonType> make_default_json_registry() {
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
  reg.template add<std::vector<std::size_t>>();
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
    return m_registry.read(tid, adapter::at(m_json, key), std::string{key}, qualified(key));
  }

  std::string qualified(const KeyType &key) const { return detail::join_path(m_path, std::string{key}); }

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
  /// the base of every error path.
  std::string path{};
  /// Keys of the document format itself, never reported as unknown.
  std::vector<std::string> reserved_keys{"type", "name"};
  /// Receives warnings as (path, message); empty: printed to stderr.
  std::function<void(const std::string &, const std::string &)> on_warning{};
};

/// Convert a JSON object into a validated parameter_handler: every
/// parameter of the schema is read (reader registry), defaults applied and
/// checks run. Every input error is a json_conversion_error carrying the
/// path of the parameter, or of the object for schema checks.
template <typename JsonType, typename KeyType, typename ParameterHandler>
void json_to_parameters(const JsonType &json,
                        const input_parameter_controller<KeyType, ParameterHandler> &schema,
                        ParameterHandler &params, const json_reader_registry<JsonType> &registry,
                        const json_conversion_options &options = {}) {
  using adapter = json_adapter<JsonType>;
  json_parameter_visitor<JsonType, KeyType> visitor(json, registry, options.path);

  if (!adapter::is_object(json))
    throw json_conversion_error(options.path, "expected a JSON object, got " + adapter::type_name(json));

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
      auto const where{visitor.qualified(key)};
      if (options.unknown_keys == unknown_key_policy::error)
        throw json_conversion_error(where, "unknown parameter");
      if (options.on_warning)
        options.on_warning(where, "unknown parameter (not in the schema)");
      else
        println(stderr, "  warning: {}: unknown parameter (not in the schema)", where);
    }
  }

  try {
    schema.accept(visitor, params);
  } catch (const json_conversion_error &) {
    throw;
  } catch (const std::invalid_argument &e) {
    // schema checks (missing, range) know the parameter, not the document
    throw json_conversion_error(options.path, e.what());
  } catch (const std::runtime_error &e) {
    throw json_conversion_error(options.path, e.what());
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
