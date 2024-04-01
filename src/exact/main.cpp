#include "../pace_graph/order.hpp"
#include "exact_solver.h"
#include <iostream>

int main() {
    //PaceGraph graph = PaceGraph::from_gr(std::cin);
    PaceGraph graph = PaceGraph::from_file("../data/heuristic_public/1.gr");
    ExactSolver solver;
    solver.solve(graph);
    return 0;
}
