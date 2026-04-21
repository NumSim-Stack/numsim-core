#ifndef FUNCTION_REGISTRY_H
#define FUNCTION_REGISTRY_H

#include <tuple>
#include <unordered_map>
#include <string>
#include <functional>
#include <exception>

namespace numsim_core {

template<typename... Signatures>
class function_registry {
public:
  // Each signature has a map: name -> std::function<Signature>
  using RegistryTuple = std::tuple<std::unordered_map<std::string, std::function<Signatures>>...>;

  function_registry() = default;

         // Register a function with a specific signature
  template<typename Signature>
  void set_function(const std::string& name, std::function<Signature> func) {
    auto& map = getMap<Signature>();
    map[name] = std::move(func);
  }

         // Overload for raw functions or lambdas
  template<typename Signature, typename F>
  void set_function(const std::string& name, F&& func) {
    set_function<Signature>(name, std::function<Signature>(std::forward<F>(func)));
  }

  template<typename Signature>
  auto get_function(const std::string& name) {
    auto& map{getMap<Signature>()};
    return map[name];
  }

  template<typename Signature>
  bool contains(const std::string& name) const {
    const auto& map{getMap<Signature>()};
    auto it{map.find(name)};
    return it != map.end();
  }

         // Call a function by name, passing the correct arguments
  template<typename Signature, typename... Args>
  auto call(const std::string& name, Args&&... args) -> decltype(auto) {
    auto& map = getMap<Signature>();
    auto it = map.find(name);
    if (it == map.end()) {
      throw std::runtime_error("Function '" + name + "' not found for signature.");
    }
    return it->second(std::forward<Args>(args)...);
  }

private:
  RegistryTuple registries;

  template<typename Signature>
  auto& getMap() {
    return std::get<std::unordered_map<std::string, std::function<Signature>>>(registries);
  }

  template<typename Signature>
  const auto& getMap() const {
    return std::get<std::unordered_map<std::string, std::function<Signature>>>(registries);
  }
};


}

#endif // FUNCTION_REGISTRY_H
