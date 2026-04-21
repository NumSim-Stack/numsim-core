#ifndef NUMSIM_CORE_PG_PROPERTY_REGISTRY_H
#define NUMSIM_CORE_PG_PROPERTY_REGISTRY_H

#include <string>
#include <unordered_map>
#include <numsim-core/property_graph/property.h>

namespace numsim_core {

class property_registry {
public:
  property_registry() {}
  auto& data() { return m_data; }

private:
  std::unordered_map<std::string, std::unordered_map<std::string, property_base_ptr<property_traits>>> m_data;
};

} // namespace numsim_core

#endif // NUMSIM_CORE_PG_PROPERTY_REGISTRY_H
