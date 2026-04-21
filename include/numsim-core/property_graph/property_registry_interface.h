#ifndef NUMSIM_CORE_PG_PROPERTY_REGISTRY_INTERFACE_H
#define NUMSIM_CORE_PG_PROPERTY_REGISTRY_INTERFACE_H

#include <optional>
#include <set>
#include <string>
#include <numsim-core/property_graph/property.h>
#include <numsim-core/property_graph/history_property.h>

namespace numsim_core {

/// Wraps a shared property registry and tracks which properties
/// this material has produced. One instance per material.
template <typename Registry, typename Traits = property_traits>
class property_registry_interface {
public:
  property_registry_interface(Registry& reg) : m_reg(reg) {}

  template <typename T>
  T& add_property(std::string const& owner, std::string const& name, T&& value) {
    if (auto existing = find(owner, name)) {
      m_produced_properties.insert(*existing);
      return static_cast<property<T, property_traits>&>(**existing).get();
    }
    property_traits traits;
    traits.id.name = name;
    traits.id.owner = owner;
    auto prop = make_property<T>(std::forward<T>(value), std::move(traits));
    auto& data = prop.get()->get();
    m_produced_properties.insert(prop.get());
    m_reg.data()[std::string(owner)][std::string(name)] = std::move(prop);
    return data;
  }

  template <typename T>
  history_property<T>& add_history(std::string const& owner,
                                   std::string const& name, T&& value) {
    if (auto existing = find(owner, name)) {
      m_produced_properties.insert(*existing);
      return static_cast<history_property<T>&>(**existing);
    }
    property_traits traits;
    traits.id.name = name;
    traits.id.owner = owner;
    auto prop = make_history_property<T>(
        std::forward<T>(value), std::forward<T>(value), std::move(traits));
    auto& data = *prop.get();
    m_produced_properties.insert(prop.get());
    m_reg.data()[std::string(owner)][std::string(name)] = std::move(prop);
    return data;
  }

  std::optional<property_base<property_traits>*> find(std::string const& owner,
                                     std::string const& name) noexcept {
    auto pos = m_reg.data().find(owner);
    if (pos != m_reg.data().end()) {
      auto pos_inner = pos->second.find(name);
      if (pos_inner != pos->second.end())
        return pos_inner->second.get();
    }
    return std::nullopt;
  }

  const std::set<property_base<property_traits>*>& produced_properties() const noexcept {
    return m_produced_properties;
  }

private:
  Registry& m_reg;
  std::set<property_base<property_traits>*> m_produced_properties;
};

} // namespace numsim_core

#endif // NUMSIM_CORE_PG_PROPERTY_REGISTRY_INTERFACE_H
