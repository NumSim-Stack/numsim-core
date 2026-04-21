#ifndef NUMSIM_CORE_PG_PROPERTY_TRAITS_H
#define NUMSIM_CORE_PG_PROPERTY_TRAITS_H

#include <functional>
#include <string>
#include <vector>

namespace numsim_core {

enum class EdgeKind : uint8_t {
  Global,
  Local
};

struct property_id {
  std::string name;
  std::string owner;
};

struct dependency {
  property_id id;
  EdgeKind kind = EdgeKind::Global;
};

struct property_traits {
  property_id id;
  std::function<void()> update;
  std::vector<dependency> input_dependencies;
};

} // namespace numsim_core

#endif // NUMSIM_CORE_PG_PROPERTY_TRAITS_H
