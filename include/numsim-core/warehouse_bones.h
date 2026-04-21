#ifndef WAREHOUSE_BONES_H
#define WAREHOUSE_BONES_H

#include <unordered_map>
#include <string>
#include <memory>

namespace numsim_core {
namespace warehouse_detail {

template<typename Key, typename Type>
void insert(std::unordered_map<Key, Type> & data, Key const& key, Type && object){
  data[key] = std::move(object);
}
}

template <template<typename>class Container, typename ValueType>
class warehouse
{
public:
  using container = Container<ValueType>;
    warehouse() {}
  warehouse(warehouse const&) = delete;

    template<typename ...Args>
  void insert(ValueType && value_type, Args &&...args){
    warehouse_detail::insert(m_data, std::forward<Args>(args)...);
  }
private:
  container m_data;
};

}
#endif // WAREHOUSE_BONES_H
