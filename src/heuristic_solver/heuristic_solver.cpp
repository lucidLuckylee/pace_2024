#include "heuristic_solver.hpp"
#include "genetic_algorithm.hpp"

Order HeuristicSolver::run(PaceGraph &graph) {
    GeneticHeuristic geneticHeuristic(
        [this]() { return this->has_time_left(); });
    return geneticHeuristic.solve(graph);
}
