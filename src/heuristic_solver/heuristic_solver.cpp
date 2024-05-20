#include "heuristic_solver.hpp"
#include "genetic_algorithm.hpp"
#include "mean_position_heuristic.hpp"

Order HeuristicSolver::largeGraphHeuristic(PaceGraph &graph) {
    std::vector<int> positionOrder(graph.size_free);
    for (int i = 0; i < graph.size_free; ++i) {
        positionOrder[i] = i;
    }

    MeanPositionParameter meanPositionParameter;
    meanPositionParameter.meanType = average;
    MeanPositionSolver meanPositionSolver1(
        [this](int it) {
            return it == 0 && this->time_percentage_past() < 0.2;
        },
        meanPositionParameter);

    Order o1 = meanPositionSolver1.solve(graph);
    meanPositionParameter.meanType = median;
    MeanPositionSolver meanPositionSolver2(
        [this](int it) {
            return it == 0 && this->has_time_left() &&
                   this->time_percentage_past() < 0.5;
        },
        meanPositionParameter);

    Order o2 = meanPositionSolver2.solve(graph);

    long cost1 = o1.count_crossings(graph);
    long cost2 = o2.count_crossings(graph);

    Order bestOrder = cost1 < cost2 ? o1 : o2;

    bool foundImprovement = true;

    long largestFallback = 20000;
    int largestMoveDistance = 2000;
    while (foundImprovement) {
        foundImprovement = false;
        if (!has_time_left()) {
            break;
        }

        std::shuffle(positionOrder.begin(), positionOrder.end(),
                     std::mt19937(std::random_device()()));
        for (int i = 0; i < graph.size_free; i++) {

            if (!has_time_left()) {
                break;
            }

            int v = positionOrder[i];
            int posOfV = bestOrder.get_position(v);

            int bestCostChange = 0;
            int bestPos = posOfV;
            int currentCostChange = 0;
            bool foundBestPos = true;
            long currentFallback = 0;

            for (int pos = posOfV - 1;
                 pos >= std::max(0, posOfV - largestMoveDistance); pos--) {
                if (!has_time_left()) {
                    foundBestPos = false;
                    break;
                }
                int u = bestOrder.get_vertex(pos);

                auto [u_v, v_u] = graph.calculatingCrossingNumber(u, v);
                int crossingDiff = v_u - u_v;
                currentCostChange += crossingDiff;

                if (currentCostChange >= largestFallback) {
                    break;
                }

                if (currentFallback < currentCostChange) {
                    currentFallback = currentCostChange;
                }

                if (currentCostChange <= 0) {
                    if (2 * currentFallback > largestFallback) {
                        largestFallback = 2 * currentFallback;
                        std::cerr << "Increase Fallback: " << largestFallback
                                  << std::endl;
                    }
                }

                if (currentCostChange < bestCostChange) {
                    foundImprovement = true;
                    bestCostChange = currentCostChange;
                    bestPos = pos;
                }
            }

            currentCostChange = 0;
            currentFallback = 0;

            for (int pos = posOfV + 1;
                 pos < std::min(graph.size_free, posOfV + largestMoveDistance);
                 pos++) {

                if (!has_time_left()) {
                    foundBestPos = false;
                    break;
                }

                int u = bestOrder.get_vertex(pos);

                auto [u_v, v_u] = graph.calculatingCrossingNumber(u, v);
                int crossingDiff = -v_u + u_v;
                currentCostChange += crossingDiff;

                if (currentCostChange >= largestFallback) {
                    break;
                }

                if (currentFallback < currentCostChange) {
                    currentFallback = currentCostChange;
                }

                if (currentCostChange <= 0) {
                    if (2 * currentFallback > largestFallback) {
                        largestFallback = 2 * currentFallback;
                        std::cerr << "Increase Fallback: " << largestFallback
                                  << std::endl;
                    }
                }

                if (currentCostChange < bestCostChange) {
                    bestCostChange = currentCostChange;
                    bestPos = pos;
                    foundImprovement = true;
                }
            }

            if (bestPos != posOfV && foundBestPos) {
                bestOrder.move_vertex(v, bestPos);
            }
        }
    }

    return bestOrder;
}

Order HeuristicSolver::run(PaceGraph &graph) {

    bool canInitCrossingMatrix = graph.init_crossing_matrix_if_necessary();
    if (canInitCrossingMatrix) {

        GeneticHeuristic geneticHeuristic(
            [this](int it) { return this->has_time_left(); });
        return geneticHeuristic.solve(graph);
    }

    return largeGraphHeuristic(graph);
}
