#include "local_search.hpp"

#include <algorithm>
#include <chrono>
#include <random>

/**
 * This function is used to move a vertex v to the best position in the order,
 * when every other vertex is fixed.
 *
 * @return the cost change of the move (should be negative or zero)
 */
int sifting_node(PaceGraph &graph, Order &order, int v) {
    int posOfV = order.get_position(v);
    int bestPositionToInsert = posOfV;
    int bestCostChange = 0;

    int crossingOld = 0;
    auto crossing_matrix_diff = graph.crossing_matrix_diff[v];
    for (int i = posOfV - 1; i >= 0; i--) {
        int u = order.get_vertex(i);

        crossingOld += crossing_matrix_diff[u];

        if (crossingOld < bestCostChange) {
            bestCostChange = crossingOld;
            bestPositionToInsert = i;
        }
    }

    crossingOld = 0;
    for (int i = posOfV + 1; i < graph.size_free; ++i) {
        int u = order.get_vertex(i);
        crossingOld -= crossing_matrix_diff[u];

        if (crossingOld < bestCostChange) {
            bestCostChange = crossingOld;
            bestPositionToInsert = i;
        }
    }

    order.move_vertex(v, bestPositionToInsert);
    return bestCostChange;
}

int sifting(PaceGraph &graph, Order &order, LocalSearchParameter &parameter,
            std::vector<int> &position_array) {

    if (parameter.siftingType == SiftingType::None) {
        return 0;
    }

    if (parameter.siftingType == SiftingType::Random) {
        unsigned seed =
            std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(position_array.begin(), position_array.end(),
                     std::default_random_engine(seed));
    } else if (parameter.siftingType == SiftingType::DegreeOrder) {
        std::sort(position_array.begin(), position_array.end(),
                  [&graph](int u, int v) {
                      return graph.neighbors_free[u].size() <
                             graph.neighbors_free[v].size();
                  });
    }

    int improvement = 0;
    for (int v = 0; v < graph.size_free; v++) {
        improvement += sifting_node(graph, order, v);
    }
    return improvement;
}

int local_search(PaceGraph &graph, Order &order,
                 LocalSearchParameter &parameter) {
    int improvement = 0;

    std::vector<int> position_array;
    position_array.reserve(graph.size_free);
    for (int i = 0; i < graph.size_free; ++i) {
        position_array.push_back(i);
    }

    improvement += sifting(graph, order, parameter, position_array);

    // Create a vector with all the positions
    // This will be used to shuffle the order of the positions in the free set
    // With this two vertices are not always checked in the same order

    for (int v = 0; v < graph.size_free; v++) {
        improvement += sifting_node(graph, order, v);
    }

    // Do this while loop, while there is still improvement
    /*bool found_improvement = true;
    while (found_improvement) {
        found_improvement = false;

        unsigned seed =
            std::chrono::system_clock::now().time_since_epoch().count();

        std::shuffle(position_array.begin(), position_array.end(),
                     std::default_random_engine(seed));

        // Go over every two vertices in the free set
        for (int pos1 = 0; pos1 < graph.size_free; ++pos1) {
            for (int pos2 = pos1 + 1; pos2 < graph.size_free; ++pos2) {
                int cost_change =
                    order.cost_change_if_swap_position(pos1, pos2, graph);

                // If the swap is beneficial, do it
                if (cost_change < 0) {
                    order.swap_by_position(pos1, pos2);
                    improvement += cost_change;
                    found_improvement = true;
                }
            }
        }
    }*/

    return improvement;
}