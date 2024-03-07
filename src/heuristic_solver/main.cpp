// heuristic_solver/main.cpp
#include "../pace_graph/order.hpp"
#include "../pace_graph/segment_tree.hpp"
#include "cheap_heuristics.hpp"
#include "genetic_algorithm.hpp"
#include <iostream>
#include <ostream>

int main() {
    std::cout << "# Reading in graph" << std::endl;
    PaceGraph graph_from_file =
        // PaceGraph::from_file("data/heuristic_public/99.gr");
        PaceGraph::from_gr(std::cin);

    MeanPositionSolver *solver =
        new MeanPositionSolver(graph_from_file, MeanTypeAlgo::median);
    Order sol = solver->solve();

    // Order sol = genetic_algorithm(graph_from_file, 10);

    long crossings = sol.count_crossings(graph_from_file);
    std::cout << "# Crossings: " << crossings << std::endl;
    std::cout << sol.convert_to_real_node_id(graph_from_file);

    return 0;
}
