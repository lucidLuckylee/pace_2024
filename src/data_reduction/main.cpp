#include "../pace_graph/pace_graph.hpp"
#include "../data_reduction/data_reduction_rules.hpp"
#include "../heuristic_solver/greedy_insert_solver.h"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    graph.init_crossing_matrix_if_necessary();
    // Get an upper bound via greedy insert heuristic
    GreedyInsertSolver upper_bound_solver([](int useless_param) { return true; });
    Order upper_bound_order = upper_bound_solver.solve(graph);
    graph.ub = upper_bound_order.count_crossings(graph);
    apply_reduction_rules(graph);
    std::cout << graph.to_gr() << std::endl;

    // TODO Reconstruct correct order solution so we can check correctness
    return 0;
}
