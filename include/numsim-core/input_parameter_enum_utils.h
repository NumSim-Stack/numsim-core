#ifndef INPUT_PARAMETER_ENUM_UTILS_H
#define INPUT_PARAMETER_ENUM_UTILS_H

#include <unordered_map>
#include <string>
#include <stdexcept>

namespace numsim_core {
template<typename Enum>
struct enum_traits;
}

// Macro to define string → enum and (optional) enum → string maps
#define NUMSIM_CORE_DEFINE_ENUM_TRAITS(EnumType, ...)                                       \
template<>                                                                      \
struct numsim_core::enum_traits<EnumType> {                                                  \
      static const std::unordered_map<std::string, EnumType>& str_to_enum() {     \
        static const std::unordered_map<std::string, EnumType> map = __VA_ARGS__;\
        return map;                                                             \
  }                                                                           \
      static const std::unordered_map<EnumType, std::string>& enum_to_str() {     \
        static std::unordered_map<EnumType, std::string> rev;                   \
        if (rev.empty()) {                                                      \
          for (const auto& [k, v] : str_to_enum()) {                          \
            rev[v] = k;                                                     \
      }                                                                   \
    }                                                                       \
        return rev;                                                             \
  }                                                                           \
      static bool is_valid(const std::string& key) {                              \
        return str_to_enum().count(key) > 0;                                    \
  }                                                                           \
      static EnumType from_string(const std::string& key) {                       \
        auto it = str_to_enum().find(key);                                      \
        if (it == str_to_enum().end()) {                                        \
          throw std::invalid_argument("Invalid enum value: " + key);          \
    }                                                                       \
        return it->second;                                                      \
  }                                                                           \
      static std::string to_string(EnumType value) {                              \
        auto it = enum_to_str().find(value);                                    \
        if (it == enum_to_str().end()) {                                        \
          throw std::invalid_argument("Invalid enum instance");               \
    }                                                                       \
        return it->second;                                                      \
  }                                                                           \
};


#endif // INPUT_PARAMETER_ENUM_UTILS_H
