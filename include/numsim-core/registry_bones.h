#ifndef REGISTRY_BONES_H
#define REGISTRY_BONES_H

#include <memory>
#include <string>
#include <map>
#include <stdexcept>
#include <memory>


namespace numsim_core {

#define combineNamesImpl(X, Y) X##Y
#define combineNames(X, Y) combineNamesImpl(X, Y)


#define RegisterObject(Registry, Name, ObjectType) \
    static auto _##Registry##_##Name = Registry::add_object<ObjectType>(#Name);
    //static auto combineNames(_##Registry##_##Name_, __COUNTER__) = Registry::add_object<ObjectType>(#Name);


template<template<class...> class Map,
         typename Key,
         typename Entry>
class registry
{
public:
    using key_type = Key;
    using mapped_type = Entry;
    using element_type = Entry;
    using pointer = typename Entry::pointer;
    using build_ptr = typename Entry::build_ptr;
    using entry_type = Entry;
    //Pointer<BaseType>(*)(Args...);

    registry(registry const &) = delete;

    registry(registry &&) = delete;

    registry & operator=(registry &&) = delete;

    registry & operator=(registry const &) = delete;

    static registry & get(){
        static registry rig;
        return rig;
    }

    template<typename T>
    static constexpr inline char add_object(key_type const& name){
        auto entry = std::make_unique<Entry>();
        entry->template setup<T>(name, &build_func<T>);
//        entry.m_build_ptr = &build_func<T>;
//        entry.m_name = name;
        get().m_entries[name] = std::move(entry);
        return '0';
    }

    constexpr inline auto begin(){
        return m_entries.begin();
    }

    constexpr inline auto end(){
        return m_entries.end();
    }

    static constexpr inline entry_type const& entity(key_type const& name){
        auto pos{get().m_entries.find(name)};
        if(pos == get().m_entries.end()){
            throw std::runtime_error("uvwBase::registry::build() "+name+" is not a valid input");
        }
        return *(pos->second).get();
    }

    template<typename ...Args>
    static constexpr inline auto build(key_type const& name, Args... args){
        auto pos{get().m_entries.find(name)};
        if(pos == get().m_entries.end()){
            throw std::runtime_error("uvwBase::registry::build() "+name+" is not a valid input");
        }
        return std::move(get().m_entries[name]->build(args...));
    }

    static constexpr inline auto erase(key_type const& name){
        get().m_entries.erase(name);
    }

private:

    template<typename Derived, typename ...Args>
    static pointer build_func(Args... args){
        return std::move(pointer(new Derived(args...)));
    }

    registry(){}

    Map<Key, std::unique_ptr<Entry>> m_entries;
};

}
#endif // REGISTRY_BONES_H
