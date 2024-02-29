#include "genetic_algorithm.h"
#include "cheap_heuristics.hpp"
#include "local_search.h"
#include <iostream>

/**
 * @param graph get a
 * @param time_limit
 * @return
 */
Order genetic_algorithm(PaceGraph &graph, int time_limit) {
    graph.init_crossing_matrix_if_necessary();

    Order bestOrder(graph.size_free);
    int bestCost = bestOrder.count_crossings(graph);

    int start = time(0);

    int number_of_iterations = 0;
    while (time(0) - start < time_limit) {
        // Order newOrder = mean_position_algorithm(graph, median);
        Order newOrder(graph.size_free);
        newOrder.permute();

        local_search(graph, newOrder);

        int newCost = newOrder.count_crossings(graph);
        if (newCost < bestCost) {
            bestOrder = newOrder;
            bestCost = newCost;
        }
        number_of_iterations++;
    }

    std::cout << "# Heuristic finished with: " << number_of_iterations
              << " iterations" << std::endl;

    return bestOrder;
}
