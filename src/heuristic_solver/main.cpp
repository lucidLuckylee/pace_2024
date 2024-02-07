// heuristic_solver/main.cpp
#include "../pace_graph/order.hpp"
#include "genetic_algorithm.h"
#include <iostream>

int main() {
    PaceGraph graph_from_file = PaceGraph::from_file("../data/website_20.gr");

    Order sol = genetic_algorithm(graph_from_file, 1);

    int crossings = sol.count_crossings(graph_from_file);
    std::cout << "# Crossings: " << crossings << std::endl;
    std::cout << sol.to_string();

    return 0;
}
