#include "../pace_graph/order.hpp"
#include "cp_solver.hpp"
#include "ilp_solver.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    ILPSolver solver;
    solver.solve(graph);
    return 0;
}