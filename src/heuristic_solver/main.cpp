#include "../pace_graph/order.hpp"
#include "genetic_algorithm.hpp"
#include "heuristic_solver.hpp"
#include <iostream>

int main(int argc, char **argv) {
    HeuristicSolver solver;
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    GeneticHeuristicParameter::getInstance().parseArgs(argc, argv);
    solver.solve(graph);
    return 0;
}
