//
// Created by jt on 10.03.24.
//

#include "simple_lb.hpp"
#include <algorithm>
#include <iostream>
#include <random>

int improveWithPotential(PaceGraph &graph, SimpleLBParameter &parameter) {
    std::vector<std::tuple<int, int, int>> conflictPairs;

    for (int u = 0; u < graph.size_free; ++u) {

        for (int v = u; v < graph.size_free; ++v) {
            if (u == v)
                continue;
            if (graph.crossing_matrix_diff[u][v] < 0) {
                for (int w = v; w < graph.size_free; w++) {
                    if (w == u || w == v)
                        continue;

                    if (graph.crossing_matrix_diff[v][w] < 0 &&
                        graph.crossing_matrix_diff[u][w] > 0) {
                        conflictPairs.emplace_back(u, v, w);
                    }
                }
            }
        }
    }

    if (conflictPairs.empty()) {
        return 0;
    }

    int bestLBImprovement = 0;

    for (int _ = 0; _ < parameter.numberOfIterationsForConflictOrder; _++) {
        std::shuffle(conflictPairs.begin(), conflictPairs.end(),
                     std::default_random_engine(_));

        std::vector<std::vector<int>> potentialMatrix(
            graph.crossing_matrix_diff.size());

        for (int i = 0; i < graph.crossing_matrix_diff.size(); i++) {
            potentialMatrix[i].resize(graph.crossing_matrix_diff.size());
            for (int j = 0; j < graph.crossing_matrix_diff.size(); ++j) {
                potentialMatrix[i][j] = graph.crossing_matrix_diff[i][j];
            }
        }

        int currentLBImprovement = 0;

        for (auto [u, v, w] : conflictPairs) {
            if (potentialMatrix[u][v] < 0 && potentialMatrix[v][w] < 0 &&
                potentialMatrix[u][w] > 0) {
                int minValue = std::min(
                    std::min(-potentialMatrix[u][v], -potentialMatrix[v][w]),
                    potentialMatrix[u][w]);

                currentLBImprovement += minValue;
                potentialMatrix[u][v] += minValue;
                potentialMatrix[v][w] += minValue;
                potentialMatrix[u][w] -= minValue;

                potentialMatrix[v][u] -= minValue;
                potentialMatrix[w][v] -= minValue;
                potentialMatrix[w][u] += minValue;
            }
        }

        if (currentLBImprovement > bestLBImprovement) {
            bestLBImprovement = currentLBImprovement;
        }
    }

    return bestLBImprovement;
}

int simpleLB(PaceGraph &graph, SimpleLBParameter &parameter) {
    graph.init_crossing_matrix_if_necessary();
    int lb = 0;

    for (int u = 0; u < graph.size_free; ++u) {
        for (int v = u + 1; v < graph.size_free; v++) {
            lb += std::min(graph.crossing_matrix[u][v],
                           graph.crossing_matrix[v][u]);
        }
    }

    if (parameter.usePotentialMatrix) {
        lb += improveWithPotential(graph, parameter);
    }

    return lb;
}