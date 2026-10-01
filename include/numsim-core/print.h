#ifndef NUMSIM_CORE_PRINT_H
#define NUMSIM_CORE_PRINT_H

#include <cstdio>
#include <format>
#include <ostream>
#include <string>
#include <utility>

namespace numsim_core {

namespace detail {
inline void write(std::FILE* stream, std::string const& text) {
  std::fwrite(text.data(), 1, text.size(), stream);
}
inline void write(std::ostream& stream, std::string const& text) { stream << text; }
}  // namespace detail

template <typename... Args>
void print(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
  detail::write(stream, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void print(std::ostream& stream, std::format_string<Args...> fmt, Args&&... args) {
  detail::write(stream, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) {
  numsim_core::print(stdout, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void println(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
  std::string text = std::format(fmt, std::forward<Args>(args)...);
  text.push_back('\n');
  detail::write(stream, text);
}

template <typename... Args>
void println(std::ostream& stream, std::format_string<Args...> fmt, Args&&... args) {
  std::string text = std::format(fmt, std::forward<Args>(args)...);
  text.push_back('\n');
  detail::write(stream, text);
}

template <typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args) {
  numsim_core::println(stdout, fmt, std::forward<Args>(args)...);
}

inline void println(std::FILE* stream = stdout) { std::fputc('\n', stream); }
inline void println(std::ostream& stream) { stream << '\n'; }

}  // namespace numsim_core

#endif  // NUMSIM_CORE_PRINT_H
