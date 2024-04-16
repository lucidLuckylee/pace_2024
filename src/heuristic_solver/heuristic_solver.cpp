#include "heuristic_solver.hpp"
#include "genetic_algorithm.hpp"
#include "genetic_algo.hpp"
#include "mean_position_heuristic.hpp"

Order HeuristicSolver::run(PaceGraph &graph) {

    bool canInitCrossingMatrix = graph.init_crossing_matrix_if_necessary();
    if (canInitCrossingMatrix) {
        GeneticAlgoSolver genetic_algo_solver(
            [this](int it) { return this->has_time_left(); });
        return genetic_algo_solver.solve(graph);
    }

    MeanPositionParameter cheapHeuristicsParameter;
    MeanPositionSolver meanPositionSolver(
        [this](int it) { return this->has_time_left(); },
        cheapHeuristicsParameter);

    return meanPositionSolver.solve(graph);
}
