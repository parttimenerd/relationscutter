#include <iostream>
#include <cxxopts.hpp>
#include <fstream>
#include <filesystem>
#include "graph.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bundled/ranges.h"

void process(std::istream &input, cxxopts::ParseResult options) {
  std::unique_ptr<RelationsGraph> rg = RelationsGraph::parse(input);
  typedef std::vector<std::string> s;
  auto ins = rg->get_variables(options["input"].as<s>());
  auto outs = rg->find_ind_vars(options["method"].as<std::string>(), options["output"].as<s>());
  if (ins.empty()) {
    spdlog::warn("no input variables found");
  }
  if (outs.empty()) {
    spdlog::warn("no output variables found");
  }
  spdlog::info("input variable: {} ", fmt::join(ins, ", "));
  spdlog::info("output variable: {} ", fmt::join(outs, ", "));
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
                  ("o,output", "output variable prefixes",
                   cxxopts::value<std::vector<std::string>>()->default_value({}))
                          ("m,method", "output method, that the output variables have to be related to", cxxopts::value<std::string>()->default_value("main"))
                          ("rel_cnf", "", cxxopts::value<std::string>())
                                          ("v,verbose", "enable some debug output", cxxopts::value<bool>()->default_value("false"))
                                                  ("d,debug", "enable all debug output", cxxopts::value<bool>()->default_value("false"))
                          ("h,help", "Print usage");
  options.parse_positional({"rel_cnf"});
  options.positional_help("cnf file, uses stdin if omitted");
  auto result = options.parse(argc, argv);
  if (result.count("help"))
  {
    std::cout << options.help() << std::endl;
    exit(0);
  }
  spdlog::set_level(spdlog::level::warn);
  if (result["verbose"].as<bool>()) {
    spdlog::set_level(spdlog::level::info);
  }
  if (result["debug"].as<bool>()) {
    spdlog::set_level(spdlog::level::debug);
  }
  if (result["rel_cnf"].count() == 0) {
    process(std::cin, result);
  } else {
    auto cnf_file = result["rel_cnf"].as<std::string>();
    if (!std::filesystem::exists(cnf_file)) {
      spdlog::error("file {} does not exist", cnf_file);
      std::exit(1);
    }
    std::ifstream input(cnf_file);
    process(input, result);
  }
  return 0;
}