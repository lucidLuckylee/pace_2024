#include "local_search.h"

#include <algorithm>
#include <random>

int local_search(PaceGraph &graph, Order &order) {
    int improvement = 0;

    bool found_improvement = true;

    std::vector<int> position_array;

    position_array.reserve(graph.size_free);
    for (int i = 0; i < graph.size_free; ++i) {
        position_array.push_back(i);
    }

    while (found_improvement) {
        found_improvement = false;

        unsigned seed =
            std::chrono::system_clock::now().time_since_epoch().count();

        std::shuffle(position_array.begin(), position_array.end(),
                     std::default_random_engine(seed));

        for (int pos1 = 0; pos1 < graph.size_free; ++pos1) {
            for (int pos2 = pos1 + 1; pos2 < graph.size_free; ++pos2) {
                int cost_change =
                    order.cost_change_if_swap_position(pos1, pos2, graph);

                if (cost_change < 0) {
                    order.swap_by_position(pos1, pos2);
                    improvement += cost_change;
                    found_improvement = true;
                }
            }
        }
    }

    return improvement;
}