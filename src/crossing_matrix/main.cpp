#include "../pace_graph/pace_graph.hpp"
#include "../data_reduction/data_reduction_rules.cpp"
#include "../heuristic_solver/greedy_insert_solver.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    graph.init_crossing_matrix_if_necessary();
    //GreedyInsertSolver upper_bound_solver([](int useless_param) { return true; });
    //Order upper_bound_order = upper_bound_solver.solve(graph);
    //graph.ub = upper_bound_order.count_crossings(graph);
    //apply_reduction_rules(graph);
    
    GreedyInsertSolver solver([](int useless_param) { return true; });
    Order solution = solver.solve(graph);
    graph.crossing.print_in_order(solution);

    return 0;
}
