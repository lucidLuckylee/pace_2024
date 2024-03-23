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
long sifting_node(PaceGraph &graph, Order &order,
                  LocalSearchParameter &parameter, int v) {
    int posOfV = order.get_position(v);
    int bestPositionToInsert = posOfV;
    long bestCostChange = 0;

    long crossingOld = 0;
    auto crossing_matrix_diff = graph.crossing_matrix_diff[v];

    int foundWithThisCost = 0;
    for (int i = posOfV - 1; i >= 0 && crossingOld <= INF; i--) {
        int u = order.get_vertex(i);

        int crossingDiff = crossing_matrix_diff[u];

        if (crossingDiff >= INF / 2) {
            break;
        }

        crossingOld += crossingDiff;

        if (crossingOld <= bestCostChange) {
            bool useSolution = false;
            if (crossingOld == bestCostChange) {
                if (parameter.siftingInsertionType ==
                    SiftingInsertionType::Random) {
                    foundWithThisCost++;
                    if (rand() % foundWithThisCost == 0) {
                        bestPositionToInsert = i;
                    }
                } else {
                    useSolution = true;
                }
            } else {
                foundWithThisCost = 0;
                useSolution = true;
            }

            if (useSolution) {
                bestCostChange = crossingOld;
                bestPositionToInsert = i;
            }
        }
    }

    crossingOld = 0;
    for (int i = posOfV + 1; i < graph.size_free && crossingOld <= INF; ++i) {
        int u = order.get_vertex(i);

        int crossingDiff = -crossing_matrix_diff[u];
        if (crossingDiff >= INF / 2) {
            break;
        }

        crossingOld += crossingDiff;

        if (crossingOld <= bestCostChange) {

            bool useSolution = false;
            if (crossingOld == bestCostChange) {
                if (parameter.siftingInsertionType ==
                    SiftingInsertionType::Random) {
                    foundWithThisCost++;
                    if (rand() % foundWithThisCost == 0) {
                        bestPositionToInsert = i;
                    }
                } else {
                    useSolution = true;
                }
            } else {
                foundWithThisCost = 0;
                useSolution = true;
            }

            if (useSolution) {
                bestCostChange = crossingOld;
                bestPositionToInsert = i;
            }
        }
    }

    order.move_vertex(v, bestPositionToInsert);
    return bestCostChange;
}

long sifting(PaceGraph &graph, Order &order, LocalSearchParameter &parameter,
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
    } else if (parameter.siftingType == SiftingType::DegreeOrderReverse) {
        std::sort(position_array.begin(), position_array.end(),
                  [&graph](int u, int v) {
                      return graph.neighbors_free[u].size() >
                             graph.neighbors_free[v].size();
                  });
    }

    long improvement = 0;
    for (int v = 0; v < graph.size_free; v++) {
        improvement += sifting_node(graph, order, parameter, v);
    }
    return improvement;
}

long local_search(PaceGraph &graph, Order &order,
                  LocalSearchParameter &parameter,
                  const std::function<bool()> &has_time_left) {
    long improvement = 0;

    std::vector<int> position_array;
    position_array.reserve(graph.size_free);
    for (int i = 0; i < graph.size_free; ++i) {
        position_array.push_back(i);
    }

    bool improved = true;
    while (improved && has_time_left()) {
        long i = sifting(graph, order, parameter, position_array);
        improvement += i;

        if (i == 0) {
            improved = false;
        }

        if (!parameter.exhaustiveSifting) {
            break;
        }
    }

    return improvement;
}