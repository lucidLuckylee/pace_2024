#include "genetic_algorithm.hpp"
#include "../lb/simple_lb.hpp"
#include "cheap_heuristics.hpp"
#include "local_search.hpp"
#include <iostream>

/**
 * @param graph get a
 * @param time_limit
 * @return
 */
Order genetic_algorithm(PaceGraph &graph, int time_limit_ms) {
    SimpleLBParameter lbParameter;
    int lb = simpleLB(graph, lbParameter);
    graph.init_crossing_matrix_if_necessary();

    Order bestOrder(graph.size_free);
    int bestCost = bestOrder.count_crossings(graph);

    LocalSearchParameter parameter;
    parameter.siftingType = SiftingType::Random;

    int number_of_iterations = 0;
    int number_of_iteration_without_improvement = 0;

    auto start_time = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() - start_time <
               std::chrono::milliseconds(time_limit_ms) &&
           lb != bestCost) {

        // Order newOrder = mean_position_algorithm(graph, median);
        Order newOrder(graph.size_free);
        newOrder.permute();

        local_search(graph, newOrder, parameter);

        int newCost = newOrder.count_crossings(graph);
        if (newCost <= bestCost) {
            bestOrder = newOrder;
            bestCost = newCost;
            if (newCost < bestCost) {
                number_of_iteration_without_improvement = 0;
            }
        } else {
            number_of_iteration_without_improvement++;

            if (number_of_iteration_without_improvement > 1000) {

                newOrder = bestOrder.clone();
                for (int i = 0; i < graph.size_free - 1; i++) {

                    if (std::chrono::steady_clock::now() - start_time >
                        std::chrono::milliseconds(time_limit_ms)) {
                        break;
                    }

                    int u = newOrder.get_vertex(i);
                    int v = newOrder.get_vertex(i + 1);

                    // force node order to be different
                    graph.crossing_matrix[u][v] += 100000;
                    graph.crossing_matrix_diff[u][v] += 100000;
                    local_search(graph, newOrder, parameter);
                    graph.crossing_matrix[u][v] -= 100000;
                    graph.crossing_matrix_diff[u][v] -= 100000;

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