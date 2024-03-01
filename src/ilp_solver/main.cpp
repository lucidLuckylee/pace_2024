#include "../pace_graph/order.hpp"
#include "ilp_solver.hpp"
#include <iostream>

int main() {

    PaceGraph graph = PaceGraph::from_gr(std::cin);
    graph.init_crossing_matrix_if_necessary();
    Order sol = ilpSolver(graph);

    long crossings = sol.count_crossings(graph);
    std::cout << "# Crossings: " << crossings << std::endl;
    std::cout << sol.convert_to_real_node_id(graph);

    return 0;
}