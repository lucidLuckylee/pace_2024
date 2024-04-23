#include "../pace_graph/order.hpp"
#include "genetic_algorithm.hpp"
#include "heuristic_solver.hpp"
#include <iostream>

int main() {
    HeuristicSolver solver;
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    solver.solve(graph);
    return 0;
}
