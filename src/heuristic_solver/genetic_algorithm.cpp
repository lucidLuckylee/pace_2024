#include "genetic_algorithm.hpp"
#include "../lb/simple_lb.hpp"
#include "local_search.hpp"
#include <chrono>
#include <iostream>

Order GeneticHeuristic::solve(PaceGraph &graph) {
    if (graph.size_free <= 1) {
        return Order(graph.size_free);
    }

    graph.init_crossing_matrix_if_necessary();

    SimpleLBParameter lbParameter;
    long lb = simpleLB(graph, lbParameter);

    Order bestOrder(graph.size_free);
    long bestCost = bestOrder.count_crossings(graph);

    LocalSearchParameter localSearchParameter;
    localSearchParameter.siftingType = SiftingType::Random;

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
                geneticHeuristicParameter
                    .forceMoveAllDirectNodesAfterIterationWithNoImprovement) {

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

                    for (int i = 0; i < graph.size_free - 1; i++) {

                        if (!has_time_left(number_of_iterations)) {
                            break;
                        }

                        int pos = force_swap_position_array[i];
                        int u = newOrder.get_vertex(pos);
                        int v = newOrder.get_vertex(pos + 1);

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
