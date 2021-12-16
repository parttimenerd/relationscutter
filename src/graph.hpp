//
// Created by bechberger-local on 16.12.21.
//

#ifndef RELATIONSCUTTER_GRAPH_HPP
#define RELATIONSCUTTER_GRAPH_HPP

#include <utility>

#include "lemon/smart_graph.h"
#include "parser.hpp"
#include "lemon/hao_orlin.h"
#include "lemon/graph_to_eps.h"
#include "lemon/dimacs.h"
#include "lemon/preflow.h"

template <typename Node>
struct NodePair {
  Node start;
  Node end;
};

/** store the variable → nodes mapping */
template <typename Node>
class Variables
{
public:
  std::unordered_map<std::string, std::vector<Node>> nodes_per_variable;

  std::vector<Node> nodes(std::string variable) const {
    return nodes_per_variable.at(variable);
  }

  std::vector<Node> nodes(std::vector<std::string> variables) const {
    std::vector<Node> res;
    for (const auto &variable: variables) {
      auto nodes = nodes_per_variable.at(variable);
      res.insert(res.end(), nodes.begin(), nodes.end());
    }
    return res;
  }

  bool has_variable(std::string variable) const {
    return nodes_per_variable.find(variable) != nodes_per_variable.end();
  }
};

class RelationsGraph
{
  using Graph = lemon::SmartDigraph;
  using Node = Graph::Node;
  using NP = NodePair<Node>;
  /** id → node */
  std::vector<NP> nodes;
  Variables<NP> variables;
  /*std::vector<std::string> input_variables;
  std::vector<std::string> output_variables;*/
  Graph graph;
  Graph::ArcMap<int> capacities;

  std::optional<std::pair<Node, Node>> source_and_sink;
  const int infty = 10000000;


  void set_sources_and_sinks(std::vector<NP> inputs, std::vector<NP> outputs) {
    auto source = graph.addNode();
    auto sink = graph.addNode();
    for (const auto &input : inputs) {
      std::cout << "source -> " << graph.id(input.start) << "\n";
      capacities.set(graph.addArc(source, input.start), infty);
    }
    for (const auto &output : outputs) {
      std::cout << graph.id(output.end) << " -> sink" << "\n";
      capacities.set(graph.addArc(output.end, sink), infty);
    }
    source_and_sink = {source, sink};
  }

  void connect_aborted(parser::Aborted aborted) {
     auto inner = graph.addNode();
     for (const auto &in: variables.nodes(aborted.input)) {
       std::cout << graph.id(in.end) << " -> inner " << graph.id(inner) << "\n";
       capacities.set(graph.addArc(in.end, inner), infty);
     }
     for (const auto &out: variables.nodes(aborted.output)) {
       std::cout << "inner " << graph.id(inner) << " -> " << graph.id(out.start) << "\n";
       capacities.set(graph.addArc(inner, out.start), infty);
     }
  }

public:
  RelationsGraph(): capacities(graph) {}

  static std::unique_ptr<RelationsGraph> parse(std::istream& in) {
    auto rg = std::make_unique<RelationsGraph>();
    bool has_problem_line = false;
    for (std::string line; std::getline(in, line); ) {
      if (!has_problem_line) {
        if (parser::is_problem_line(line)) {
          auto [vars, rels] = parser::parse_problem_line(line);
          rg->nodes.reserve(vars);
          rg->graph.reserveNode(vars + 10);
          rg->graph.reserveArc(vars * 3);
          has_problem_line = true;
        }
        continue;
      }

      if (parser::is_rel_line(line)) {
        auto [to, from] = parser::parse_rel_line(line);
        for (const auto &f: from) {
          rg->graph.id(rg->node(f).end);
          rg->graph.id(rg->node(to).start);
          std::cout << f << " -> " << to << "\n";
          std::cout << "Arc(" << rg->graph.id(rg->node(f).end) << ", " << rg->graph.id(rg->node(to).start) << ")\n";
          rg->capacities.set(rg->graph.addArc(rg->node(f).end, rg->node(to).start), rg->infty);
        }
      } else if (parser::is_variable_line(line)) {
        auto [name, vars] = parser::parse_variable_line(line);
        std::vector<NP> nodes;
        for (const auto &var: vars) {
          nodes.push_back(rg->node(var));
        }
        rg->variables.nodes_per_variable.emplace(name, nodes);
      } else if (parser::is_loop_line(line)) {
        rg->connect_aborted(parser::parse_loop_line(line));
      } else if (parser::is_rec_line(line)) {
        rg->connect_aborted(parser::parse_rec_line(line));
      }

    }
    return rg;
  }

  NP node(size_t id, int weight = 1) {
    while (nodes.size() <= id) {
      auto start = graph.addNode();
      auto end = graph.addNode();
      capacities.set(graph.addArc(start, end), weight);
      std::cout << nodes.size() << ": Node(" << graph.id(start) << ", " << graph.id(end) << ")\n";
      nodes.push_back({start, end});
    }
    return nodes.at(id);
  }

  std::vector<std::string> get_variables(std::vector<std::string> prefixes) const {
    std::vector<std::string> res;
    for (const auto &prefix: prefixes) {
      for (const auto &[variable, _]: variables.nodes_per_variable) {
        if (variable.rfind(prefix, 0) == 0) {
          res.push_back(variable);
        }
      }
    }
    return res;
  }

  int compute() {
    std::ofstream os("/tmp/bla.ps");
    lemon::graphToEps(graph, os).run();
    auto [source, sink] = source_and_sink.value();
    lemon::Preflow<Graph> flow(graph, capacities, source, sink);
    for (const auto &arc: graph.arcs()) {
      std::cout << graph.id(graph.source(arc)) << " → " << graph.id(graph.target(arc)) << " = " << capacities[arc] << "\n";
    }
    std::cout << graph.id(source) << " -> " << graph.id(sink) << "\n";
    flow.run();
    return flow.flowValue();
  }

  void set_sources_and_sinks(std::vector<std::string> input_variables, std::vector<std::string> output_variables) {
    set_sources_and_sinks(variables.nodes(std::move(input_variables)), variables.nodes(std::move(output_variables)));
  }
};


#endif//RELATIONSCUTTER_GRAPH_HPP