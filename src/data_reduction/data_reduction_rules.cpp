
#include "../pace_graph/crossing_matrix.hpp"
#include "../pace_graph/pace_graph.hpp"
#include <algorithm>
#include <iostream>

bool rr1_rr2(PaceGraph &graph) {
    bool applied = false;
    for (int a = 0; a < graph.size_free; a++) {
        for (int b = a + 1; b < graph.size_free; b++) {
            // RR1
            if (graph.crossing.matrix[a][b] == 0 &&
                graph.crossing.matrix[b][a] > 0) {
                applied = graph.crossing.set_a_lt_b(a, b) || applied;
            } else if (graph.crossing.matrix[b][a] == 0 &&
                       graph.crossing.matrix[a][b] > 0) {
                applied = graph.crossing.set_a_lt_b(b, a) || applied;
            } else if (graph.neighbors_free[a] == graph.neighbors_free[b]) {
                applied = graph.crossing.set_a_lt_b(a, b) || applied;
            }
        }
    }
    return applied;
}

void apply_reduction_rules(PaceGraph &graph) {
    int k = 0;
    while (rr1_rr2(graph)) {
        std::cerr << "\rCompleted rr1_rr2 iteration " << k << std::flush;
        k += 1;
    }
}
