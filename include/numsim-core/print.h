#ifndef NUMSIM_CORE_PRINT_H
#define NUMSIM_CORE_PRINT_H

#include "namespace.h"

// std::print / std::println on top of <format>. The <print> header needs
// libstdc++ 14; <format> is already in libstdc++ 13, so this keeps the
// NumSim libraries building with GCC 13 and Clang 18 (the baseline shared
// across the stack). Semantics follow std::print: the format string is
// checked at compile time; output goes to stdout, a FILE* or a std::ostream.
//
// Calls inside this header are qualified: std::format_string is a std type,
// so an unqualified call would also find std::print via ADL when <print> is
// included elsewhere, and the two would be ambiguous.

#include <cstdio>
#include <format>
#include <ostream>
#include <string>
#include <utility>

namespace numsim::core {

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

} // namespace numsim::core

#endif  // NUMSIM_CORE_PRINT_H
