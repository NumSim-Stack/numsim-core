#include <numsim-core/print.h>

#include <gtest/gtest.h>

#include <cstdio>
#include <string>

namespace {
// Captures what numsim_core::print writes to a stream.
std::string capture(auto&& writer) {
  std::FILE* f = std::tmpfile();
  writer(f);
  std::fflush(f);
  long const size = std::ftell(f);
  std::rewind(f);
  std::string out(static_cast<std::size_t>(size), '\0');
  if (size > 0 && std::fread(out.data(), 1, out.size(), f) != out.size())
    out.clear();
  std::fclose(f);
  return out;
}
} // namespace

TEST(print, formats_like_std_format) {
  EXPECT_EQ(capture([](std::FILE* f) { numsim_core::print(f, "{}-{:.2f}", 3, 1.5); }), "3-1.50");
}

TEST(print, println_appends_a_newline) {
  EXPECT_EQ(capture([](std::FILE* f) { numsim_core::println(f, "x={}", 7); }), "x=7\n");
  EXPECT_EQ(capture([](std::FILE* f) { numsim_core::println(f); }), "\n");
}

TEST(print, no_arguments_and_braces) {
  EXPECT_EQ(capture([](std::FILE* f) { numsim_core::println(f, "{{}}"); }), "{}\n");
}

#include <sstream>

TEST(print, writes_to_ostreams) {
  std::ostringstream os;
  numsim_core::print(os, "{}", 1);
  numsim_core::println(os, "+{}", 2);
  numsim_core::println(os);
  EXPECT_EQ(os.str(), "1+2\n\n");
}

// <print> included as well: std::println must not make our calls ambiguous
// (std::format_string pulls namespace std in for ADL).
#if __has_include(<print>) && defined(__cpp_lib_print)
#include <print>
TEST(print, coexists_with_std_print) {
  EXPECT_EQ(capture([](std::FILE* f) { numsim_core::println(f, "{}", 1); }), "1\n");
  numsim_core::println("{}", "both visible");
}
#endif
