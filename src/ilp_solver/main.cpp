#include "../pace_graph/order.hpp"
#include "ilp_solver.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    ILPSolver ilpSolver;
    ilpSolver.solve(graph);
    return 0;
}