#include "lb_solver.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    LBSolver lbSolver;
    lbSolver.solve(graph);
    return 0;
}
