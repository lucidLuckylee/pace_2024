#include "genetic_algorithm.hpp"
#include "../lb/simple_lb.hpp"
#include "local_search.hpp"
#include <iostream>

Order GeneticHeuristic::solve(PaceGraph &graph) {
    SimpleLBParameter lbParameter;
    int lb = simpleLB(graph, lbParameter);
    graph.init_crossing_matrix_if_necessary();

    Order bestOrder(graph.size_free);
    int bestCost = bestOrder.count_crossings(graph);

    LocalSearchParameter localSearchParameter;
    localSearchParameter.siftingType = SiftingType::Random;

    int number_of_iterations = 0;
    int number_of_iteration_without_improvement = 0;

    auto start_time = std::chrono::steady_clock::now();

    while (has_time_left(number_of_iterations) && lb != bestCost) {
        Order newOrder(graph.size_free);
        newOrder.permute();

        local_search(graph, newOrder, localSearchParameter,
                     [this, number_of_iterations]() {
                         return has_time_left(number_of_iterations);
                     });

        int newCost = newOrder.count_crossings(graph);
        if (newCost <= bestCost) {
            bestOrder = newOrder;
            bestCost = newCost;
            if (newCost < bestCost) {
                number_of_iteration_without_improvement = 0;
            }
        } else {
            number_of_iteration_without_improvement++;
            if (number_of_iteration_without_improvement >
                geneticHeuristicParameter
                    .forceMoveAllDirectNodesAfterIterationWithNoImprovement) {

                newOrder = bestOrder.clone();
                for (int i = 0; i < graph.size_free - 1; i++) {

                    if (!has_time_left(number_of_iterations)) {
                        break;
                    }

                    int u = newOrder.get_vertex(i);
                    int v = newOrder.get_vertex(i + 1);

                    if (graph.crossing_matrix[v][u] >= INF) {
                        continue;
                    }

                    // force node order to be different
                    graph.fixNodeOrder(v, u);
                    local_search(graph, newOrder, localSearchParameter,
                                 [this, number_of_iterations]() {
                                     return has_time_left(number_of_iterations);
                                 });
                    graph.unfixNodeOrder(v, u);

                    newCost = newOrder.count_crossings(graph);

                    if (newCost <= bestCost) {
                        bestOrder = newOrder;
                        bestCost = newCost;
                    }
                }
                number_of_iteration_without_improvement = 0;
            }
        }
        number_of_iterations++;
    }

    std::cout << "# Iterations: " << number_of_iterations << std::endl;

    return bestOrder;
}
