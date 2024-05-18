#include "genetic_algorithm.hpp"
#include "../lb/simple_lb.hpp"
#include "local_search.hpp"
#include "mean_position_heuristic.hpp"
#include <chrono>
#include <iostream>

Order GeneticHeuristic::solve(PaceGraph &graph, char **argv) {
    // change varibles
    int forceMoveAllDirectNodesAfterIterationWithNoImprovement =
        std::strtol(argv[0], nullptr, 0);
    int numberOfForceSwapPositions = std::strtol(argv[1], nullptr, 0);
    int numberOfForceSwapStepSize = std::strtol(argv[2], nullptr, 0);

    if (graph.size_free <= 1) {
        return Order(graph.size_free);
    }

    LocalSearchParameter localSearchParameter;
    localSearchParameter.siftingType =
        geneticHeuristicParameter.siftingTypeInitialSearch;
    localSearchParameter.siftingInsertionType =
        geneticHeuristicParameter.siftingInsertionTypeInitialSearch;

    graph.init_crossing_matrix_if_necessary();

    SimpleLBParameter lbParameter;
    lbParameter.maxNrOfConflicts = 100000;
    long lb = simpleLB(graph, lbParameter);

    MeanPositionParameter meanPositionParameter;
    meanPositionParameter.useLocalSearch = false;
    meanPositionParameter.useJittering = false;
    meanPositionParameter.meanType = median;
    MeanPositionSolver meanPositionHeuristic(
        [this](int it) { return it == 0 && this->has_time_left(0); },
        meanPositionParameter);
    Order bestOrder = meanPositionHeuristic.solve(graph, argv);
    local_search(graph, bestOrder, localSearchParameter,
                 [this]() { return this->has_time_left(0); });
    long bestCost = bestOrder.count_crossings(graph);

    int number_of_iterations = 0;
    int number_of_iteration_without_improvement = 0;

    Order lookAtOrder = bestOrder;
    long lookAtCost = bestCost;

    std::vector<int> force_swap_position_array(graph.size_free - 1);
    for (int i = 0; i < graph.size_free - 1; ++i) {
        force_swap_position_array[i] = i;
    }

    while (has_time_left(number_of_iterations) && lb != bestCost) {
        Order newOrder(graph.size_free);
        newOrder.permute();

        local_search(graph, newOrder, localSearchParameter,
                     [this, number_of_iterations]() {
                         return has_time_left(number_of_iterations);
                     });

        long newCost = newOrder.count_crossings(graph);
        if (newCost <= lookAtCost) {
            lookAtOrder = newOrder;
            lookAtCost = newCost;
            if (newCost < lookAtCost) {
                number_of_iteration_without_improvement = 0;
            }

            if (newCost < bestCost) {
                bestOrder = newOrder;
                bestCost = newCost;
            }

        } else {
            number_of_iteration_without_improvement++;

            if (number_of_iteration_without_improvement >
                forceMoveAllDirectNodesAfterIterationWithNoImprovement) {

                localSearchParameter.siftingType =
                    geneticHeuristicParameter.siftingTypeImprovementSearch;
                localSearchParameter.siftingInsertionType =
                    geneticHeuristicParameter
                        .siftingInsertionTypeImprovementSearch;

                for (int swapFurther = 1;
                     swapFurther <=
                     numberOfForceSwapPositions;
                     swapFurther +=
                     numberOfForceSwapStepSize) {

                    if (!has_time_left(number_of_iterations)) {
                        break;
                    }

                    bool improvement = true;
                    while (improvement) {
                        improvement = false;

                        std::shuffle(force_swap_position_array.begin(),
                                     force_swap_position_array.end(),
                                     std::default_random_engine(
                                         std::chrono::system_clock::now()
                                             .time_since_epoch()
                                             .count()));
                        if (!has_time_left(number_of_iterations)) {
                            break;
                        }

                        newOrder = lookAtOrder.clone();
                        Order orginalOrder = lookAtOrder.clone();
                        for (int i = 0; i < graph.size_free - 1; i++) {

                            if (!has_time_left(number_of_iterations)) {
                                break;
                            }

                            int pos = force_swap_position_array[i];
                            if (pos + swapFurther >= graph.size_free) {
                                continue;
                            }

                            int u = orginalOrder.get_vertex(pos);
                            int v = orginalOrder.get_vertex(pos + swapFurther);

                            if (graph.crossing.lt(u, v)) {
                                continue;
                            }

                            newOrder.swap_by_vertices(u, v);

                            // force node order to be different
                            graph.crossing.set_a_lt_b(v, u);
                            local_search(graph, newOrder, localSearchParameter,
                                         [this, number_of_iterations]() {
                                             return has_time_left(
                                                 number_of_iterations);
                                         });
                            graph.crossing.unset_a_lt_b(v, u);

                            newCost = newOrder.count_crossings(graph);
                            if (newCost <= lookAtCost) {
                                lookAtOrder = newOrder;
                                if (newCost < lookAtCost) {
                                    improvement = true;
                                    lookAtCost = newCost;
                                }

                                if (newCost < bestCost) {
                                    bestOrder = newOrder;
                                    bestCost = newCost;
                                }
                            }
                        }
                    }
                }

                localSearchParameter.siftingType =
                    geneticHeuristicParameter.siftingTypeInitialSearch;
                localSearchParameter.siftingInsertionType =
                    geneticHeuristicParameter.siftingInsertionTypeInitialSearch;
                number_of_iteration_without_improvement = 0;

                lookAtOrder = Order(graph.size_free);
                lookAtOrder.permute();
                lookAtCost = lookAtOrder.count_crossings(graph);
            }
        }
        number_of_iterations++;
    }

    std::cerr << "# Iterations: " << number_of_iterations << std::endl;

    return bestOrder;
}
