#include <iostream>
#include <cxxopts.hpp>
#include <fstream>
#include <filesystem>
#include "graph.hpp"

void process(std::istream &input, cxxopts::ParseResult options) {
  std::unique_ptr<RelationsGraph> rg = RelationsGraph::parse(input);
  typedef std::vector<std::string> s;
  auto ins = rg->get_variables(options["input"].as<s>());
  auto outs = rg->get_variables(options["output"].as<s>());
  rg->set_sources_and_sinks(ins, outs);
  auto result = rg->compute();
  std::cout << "==========================\n";
  std::cout << "Leakage: " << result << "\n";
  std::cout << "==========================\n";
}

int main(int argc, char** argv) {
  cxxopts::Options options("RelationsCutter", "A tool to calculate the leakage for the __rel__ output of a modified CBMC");
  options.add_options()
                  ("i,input", "input variable prefixes", cxxopts::value<std::vector<std::string>>()->default_value({"symex::nondet"}))
                  ("o,output", "output variable prefixes", cxxopts::value<std::vector<std::string>>()->default_value({}))
                          ("rel_cnf", "", cxxopts::value<std::string>())
                          ("h,help", "Print usage");
  options.parse_positional({"rel_cnf"});
  options.positional_help("cnf file, uses stdin if omitted");
  auto result = options.parse(argc, argv);
  if (result.count("help"))
  {
    std::cout << options.help() << std::endl;
    exit(0);
  }
  if (result["rel_cnf"].count() == 0) {
    process(std::cin, result);
  } else {
    auto cnf_file = result["rel_cnf"].as<std::string>();
    if (!std::filesystem::exists(cnf_file)) {
      std::cerr << "file does not exist\n";
      std::exit(1);
    }
    std::ifstream input(cnf_file);
    process(input, result);
  }
  return 0;
}