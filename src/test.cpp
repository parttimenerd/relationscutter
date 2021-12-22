#include "parser.hpp"
#include <catch2/catch_test_macros.hpp>

template <typename T>
std::unordered_set<T> set(std::vector<T> vec) {
  return {vec.begin(), vec.end()};
}

TEST_CASE("Loop line parsed correctly", "[parser]") {
  std::string line = R"(c loop 0 main 0 -1 | sfoa 0
| guards goto_symex::\guard#1 goto_symex::\guard#2 goto_symex::\guard#3
| lguard goto_symex::\guard#4 | linput main::$tmp::return_value_f!0@2 oa_constant::$1!0
| lmisc_input f(char)::a!0@2 f(char)::a!0@1#0 main::1::h!0@1 main::1::h!0@1#2
| linner_input main::$tmp::return_value_f!0@2 main::$tmp::return_value_f!0@3#3
| linner_output main::$tmp::return_value_f!0@2 oa_constant::$2!0 f(char)::a!0@2 f(char)::a!0@1#0
| loutput main::$tmp::return_value_f!0@2 main::$tmp::return_value_f!0@4#3 f(char)::a!0@2 f(char)::a!0@1#2)";
  REQUIRE(parser::is_loop_line(line));
  auto aborted = parser::parse_loop_line(line);
  CHECK(set(aborted.output) == set(std::vector<std::string>{"main::$tmp::return_value_f!0@4#3", "f(char)::a!0@1#2"}));
  CHECK(set(aborted.input) == set(std::vector<std::string>{"oa_constant::$1!0", "f(char)::a!0@1#0", "main::1::h!0@1#2"}));
}