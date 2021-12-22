//
// Created by bechberger-local on 16.12.21.
//

#ifndef RELATIONSCUTTER_PARSER_HPP
#define RELATIONSCUTTER_PARSER_HPP


#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <unordered_set>
namespace parser {

  bool starts_with(std::string &string, std::string &&prefix) {
    return string.rfind(prefix, 0) == 0;
  }

  bool is_problem_line(std::string &line) {
    return starts_with(line, "c __rel__ p ");
  }

  bool is_rel_line(std::string &line) {
    return !is_problem_line(line) && starts_with(line, "c __rel__ ");
  }

  bool is_variable_line(std::string &line) {
    return starts_with(line, "c ") && !starts_with(line, "c __rel__ ") && !starts_with(line, "c rec ") && !starts_with(line, "c loop ");
  }

  bool is_loop_line(std::string &line) {
    return starts_with(line, "c loop ");
  }

  bool is_rec_line(std::string &line) {
    return starts_with(line, "c rec child ");
  }

  void expect(std::stringstream &ss, std::string && str) {
    std::string token;
    ss >> token;
    if (token != str) {
      throw std::exception();
    }
  }

  std::pair<int, int> parse_problem_line(std::string &line) {
    std::stringstream ss(line.substr(strlen("c __rel__ p ")));
    int var_num;
    int rel_num;
    ss >> var_num;
    ss >> rel_num;
    return {var_num, rel_num};
  }

  std::pair<size_t, std::vector<size_t>> parse_rel_line(std::string &line) {
    std::stringstream ss(line.substr(strlen("c __rel__ ")));
    size_t to;
    ss >> to;
    std::vector<size_t> from;
    size_t entry;
    while (ss >> entry) {
      from.push_back(entry);
    }
    return {to, from};
  }

  std::pair<std::string, std::vector<size_t>> parse_variable_line(std::string &line) {
    std::stringstream ss(line.substr(strlen("c ")));
    std::string name;
    std::vector<size_t> variables;
    ss >> name;
    std::string entry;
    while (ss >> entry) {
      long var;
      if (std::stringstream(entry) >> var) {
        variables.push_back(labs(var));
      }
    }
    return {name, variables};
  }

  /**
   * parse variables and return every second variable
   */
  std::vector<std::string> _parse_variables(std::stringstream ss) {
    std::vector<std::string> variables;
    for (std::string variable; (ss >> variable); ) {
      if (variable == "|") {
        break;
      }
      ss >> variable;
      variables.push_back(variable);
    }
    return variables;
  }

  std::vector<std::string> _parse_variables_part(std::string &line, std::string part){
    auto sub = line.substr(line.find("| " + part + " ") + part.length() + 3);
    return _parse_variables(std::stringstream(sub));
  }

  struct Aborted
  {
    std::vector<std::string> input;
    std::vector<std::string> output;
  };

  std::vector<std::string> combine(std::vector<std::string> first, std::vector<std::string> second) {
    std::unordered_set<std::string> res;
    res.insert(first.begin(), first.end());
    res.insert(second.begin(), second.end());
    return {res.begin(), res.end()};
  }

  Aborted parse_loop_line(std::string &line) {
    /*
    c loop [loop id] [function name] [loop nr] [parent loop or -1]
    | sfoa [1: loop can be fully over approximated, 0: not, just a hint]`
    | guards ['-' if negative][constraint var 1, conditions that are satisfied at the start of the loop] […]
    | lguard [… same but for the last (and therefore abstract) iteration]
    | linput [loop input/read variable 1] [instantiation 1] […]
    | lmisc_input [guard, uninitialized or constant variable 1] […] [just treat it as input]
    | linner_input [abstract iteration input/read variable 1] [instantiation 1] […]
    | linner_output [abstract iteration output/written variable 1] [instantiation 1] […]
    | loutput [loop output/written variable 1] [instantiation 1] […]
     */
    return {
      combine(_parse_variables_part(line, "linput"), _parse_variables_part(line, "lmisc_input")),
            _parse_variables_part(line, "loutput")
    };
  }

  Aborted parse_rec_line(std::string &line) {
    /* c rec child [id of this application] [name of the function] | input [input/read var 1] [instantiation 1] […] |
       output [output/written var 1] [instantiation 1] | constraint ['-' if negative][constraint var 1] […] */
    return {
      _parse_variables_part(line, "input"),
              _parse_variables_part(line, "output")
    };
  }
}
#endif//RELATIONSCUTTER_PARSER_HPP
