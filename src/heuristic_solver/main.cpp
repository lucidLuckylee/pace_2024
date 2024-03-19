#include "../pace_graph/order.hpp"
#include "genetic_algorithm.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    HeuristicSolver solver;
    solver.solve(graph);
    return 0;
}
