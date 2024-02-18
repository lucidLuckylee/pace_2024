// heuristic_solver/main.cpp
#include "../pace_graph/order.hpp"
#include "cheap_heuristics.hpp"
#include <iostream>
#include <ostream>


int main() {
    PaceGraph graph_from_file =
        PaceGraph::from_file("data/heuristic_public/99.gr");
        //PaceGraph::from_gr(std::cin);
    
    MeanPositionSolver* solver = new MeanPositionSolver(graph_from_file, MeanTypeAlgo::median);
    Order sol = solver->solve();

    int crossings = sol.count_crossings(graph_from_file);
    std::cout << "# Crossings: " << crossings << std::endl;
    std::cout << sol.convert_to_real_node_id(graph_from_file);

    return 0;
}
