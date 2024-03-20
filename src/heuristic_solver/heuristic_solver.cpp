#include "heuristic_solver.hpp"
#include "genetic_algorithm.hpp"
#include "median_position_heuristic.hpp"

Order HeuristicSolver::run(PaceGraph &graph) {

    bool canInitCrossingMatrix = graph.init_crossing_matrix_if_necessary();
    if (canInitCrossingMatrix) {
        GeneticHeuristic geneticHeuristic(
            [this]() { return this->has_time_left(); });
        return geneticHeuristic.solve(graph);
    }

    MeanPositionParameter cheapHeuristicsParameter;
    MeanPositionSolver meanPositionSolver(
        [this]() { return this->has_time_left(); }, cheapHeuristicsParameter);

    return meanPositionSolver.solve(graph);
}
