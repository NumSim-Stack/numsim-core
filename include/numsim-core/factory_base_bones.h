#ifndef FACTORY_BASE_BONES_H
#define FACTORY_BASE_BONES_H

#include <string>
#include <memory>
#include <vector>
#include <map>

namespace numsim_core {

template <typename Type>
class factory_base
{
public:
    factory_base():
        m_data(),
        m_data_ptr()
    {}

    ~factory_base(){}

    template<typename Derived>
    void add(std::string&& name){
        m_data[name] = std::move(std::make_unique<Derived>());
    }

    auto get(std::string name){
        return m_data.find(name)->second.get();
    }

    template<typename Derived>
    static std::unique_ptr<Type> build_func(){
        return std::make_unique<Derived>();
    }

private:
    std::map<std::string, std::unique_ptr<Type>> m_data;
    std::map<std::string, Type*> m_data_ptr;
};

}
#endif // FACTORY_BASE_BONES_H
