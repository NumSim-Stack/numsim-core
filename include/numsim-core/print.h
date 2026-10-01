#ifndef NUMSIM_CORE_PRINT_H
#define NUMSIM_CORE_PRINT_H

// std::print / std::println on top of <format>. The <print> header needs
// libstdc++ 14; <format> is already in libstdc++ 13, so this keeps the
// NumSim libraries building with GCC 13 and Clang 18 (the baseline shared
// across the stack). Semantics follow std::print: the format string is
// checked at compile time, output goes to stdout unless a FILE* is given.

#include <cstdio>
#include <format>
#include <string>
#include <utility>

namespace numsim_core {

template <typename... Args>
void print(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
  std::string const text = std::format(fmt, std::forward<Args>(args)...);
  std::fwrite(text.data(), 1, text.size(), stream);
}

template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args) {
  print(stdout, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void println(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
  std::string text = std::format(fmt, std::forward<Args>(args)...);
  text.push_back('\n');
  std::fwrite(text.data(), 1, text.size(), stream);
}

template <typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args) {
  println(stdout, fmt, std::forward<Args>(args)...);
}

inline void println(std::FILE* stream = stdout) { std::fputc('\n', stream); }

} // namespace numsim_core

#endif // NUMSIM_CORE_PRINT_H
