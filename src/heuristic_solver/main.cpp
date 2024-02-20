// heuristic_solver/main.cpp
#include "../pace_graph/order.hpp"
#include "cheap_heuristics.h"
#include "genetic_algorithm.h"
#include <iostream>

int main() {
    PaceGraph graph_from_file =
        // PaceGraph::from_file("../data/tiny/cycle_8_sorted.gr");
        PaceGraph::from_gr(std::cin);

    Order sol = mean_position_algorithm(graph_from_file, median);

    int crossings = sol.count_crossings(graph_from_file);
    std::cout << "# Crossings: " << crossings << std::endl;
    std::cout << sol.convert_to_real_node_id(graph_from_file);

    return 0;
}
