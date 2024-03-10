//
// Created by jt on 10.03.24.
//

#include "simple_lb.hpp"

int simpleLB(PaceGraph &graph) {
    int lb = 0;
    for (int u = 0; u < graph.size_free; ++u) {
        for (int v = u + 1; v < graph.size_free; v++) {
            lb += std::min(graph.crossing_matrix[u][v],
                           graph.crossing_matrix[v][u]);
        }
    }

    return lb;
}