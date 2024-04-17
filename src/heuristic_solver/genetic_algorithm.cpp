#include "genetic_algorithm.hpp"
#include "../lb/simple_lb.hpp"
#include "local_search.hpp"
#include <chrono>
#include <iostream>

Order GeneticHeuristic::solve(PaceGraph &graph) {
    graph.init_crossing_matrix_if_necessary();

    SimpleLBParameter lbParameter;
    long lb = simpleLB(graph, lbParameter);

    Order bestOrder(graph.size_free);
    long bestCost = bestOrder.count_crossings(graph);

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

        long newCost = newOrder.count_crossings(graph);
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

                    if (graph.crossing.lt(u, v)) {
                        continue;
                    }

                    newOrder.swap_by_vertices(u, v);

                    // force node order to be different
                    graph.crossing.set_a_lt_b(v, u);
                    local_search(graph, newOrder, localSearchParameter,
                                 [this, number_of_iterations]() {
                                     return has_time_left(number_of_iterations);
                                 });
                    graph.crossing.unset_a_lt_b(v, u);

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

    std::cerr << "# Iterations: " << number_of_iterations << std::endl;

    return bestOrder;
}
