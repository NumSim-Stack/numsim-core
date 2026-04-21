#ifndef NUMSIM_CORE_OBJECT_REGISTRY_H
#define NUMSIM_CORE_OBJECT_REGISTRY_H

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace numsim_core {

/// Abstract registry entry — a node that knows how to create an object
/// and optionally provide its parameter schema. Subclassed per concrete type.
///
/// @tparam Base            Polymorphic base type for created objects
/// @tparam ParameterSchema The parameter controller type, or void if no schema support
/// @tparam CtorArgs        Constructor argument types shared by all registered types
template<typename Base, typename ParameterSchema, typename... CtorArgs>
class registry_entry_base {
public:
  virtual ~registry_entry_base() = default;

  /// Create an instance of the registered type.
  virtual std::unique_ptr<Base> build(CtorArgs&... args) const = 0;

  /// Return the parameter schema for the registered type.
  virtual ParameterSchema build_parameters() const = 0;
};

/// Specialization when ParameterSchema is void — no schema support.
template<typename Base, typename... CtorArgs>
class registry_entry_base<Base, void, CtorArgs...> {
public:
  virtual ~registry_entry_base() = default;

  /// Create an instance of the registered type.
  virtual std::unique_ptr<Base> build(CtorArgs&... args) const = 0;
};

/// Typed registry entry — knows the concrete Derived type.
template<typename Derived, typename Base, typename ParameterSchema, typename... CtorArgs>
class registry_entry final : public registry_entry_base<Base, ParameterSchema, CtorArgs...> {
public:
  std::unique_ptr<Base> build(CtorArgs&... args) const override {
    return std::make_unique<Derived>(args...);
  }

  ParameterSchema build_parameters() const override {
    return Derived::parameters();
  }
};

/// Specialization when ParameterSchema is void — no build_parameters().
template<typename Derived, typename Base, typename... CtorArgs>
class registry_entry<Derived, Base, void, CtorArgs...> final
    : public registry_entry_base<Base, void, CtorArgs...> {
public:
  std::unique_ptr<Base> build(CtorArgs&... args) const override {
    return std::make_unique<Derived>(args...);
  }
};

/// Generic singleton object registry.
///
/// Stores typed registry entries keyed by string name. Each entry knows
/// how to create objects and provide their parameter schemas.
///
/// @tparam Base            The polymorphic base type
/// @tparam ParameterSchema The parameter controller type returned by build_parameters()
/// @tparam CtorArgs        The constructor argument types all registered types share
///
/// Usage:
///   using my_factory = object_registry<MyBase, MyParamController, Config&>;
///   my_factory::instance().register_type<MyDerived>("my_type");
///   auto ptr = my_factory::instance().create("my_type", config);
///   auto schema = my_factory::instance().entry("my_type").build_parameters();
template<typename Base, typename ParameterSchema, typename... CtorArgs>
class object_registry {
public:
  using entry_base = registry_entry_base<Base, ParameterSchema, CtorArgs...>;

  object_registry(const object_registry&) = delete;
  object_registry& operator=(const object_registry&) = delete;

  /// Singleton access.
  static object_registry& instance() {
    static object_registry reg;
    return reg;
  }

  /// Register a concrete type under a string key.
  template<typename Derived>
  bool register_type(const std::string& key) {
    m_entries[key] = std::make_shared<
        registry_entry<Derived, Base, ParameterSchema, CtorArgs...>>();
    return true;
  }

  /// Get the registry entry for a type (for direct access to build/build_parameters).
  const entry_base& entry(const std::string& key) const {
    auto it = m_entries.find(key);
    if (it == m_entries.end())
      throw std::runtime_error("object_registry::entry(): unknown type '" + key + "'");
    return *it->second;
  }

  /// Construct from string key + arguments.
  std::unique_ptr<Base> create(const std::string& key, CtorArgs&... args) const {
    return entry(key).build(args...);
  }

  /// Get the parameter schema for a registered type.
  /// Only available when ParameterSchema is not void.
  ParameterSchema schema(const std::string& key) const
    requires (!std::is_void_v<ParameterSchema>)
  {
    return entry(key).build_parameters();
  }

  /// Check if a type is registered.
  bool contains(const std::string& key) const {
    return m_entries.contains(key);
  }

  /// List all registered type names.
  std::vector<std::string> registered_types() const {
    std::vector<std::string> keys;
    keys.reserve(m_entries.size());
    for (auto& [k, _] : m_entries)
      keys.push_back(k);
    return keys;
  }

  /// Number of registered types.
  std::size_t size() const noexcept { return m_entries.size(); }

  /// Remove all registered types. Useful for test isolation.
  void clear() noexcept { m_entries.clear(); }

private:
  object_registry() = default;
  std::unordered_map<std::string, std::shared_ptr<entry_base>> m_entries;
};

} // namespace numsim_core

#endif // NUMSIM_CORE_OBJECT_REGISTRY_H
