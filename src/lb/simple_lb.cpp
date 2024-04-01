//
// Created by jt on 10.03.24.
//

#include "simple_lb.hpp"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <random>
#include <x86gprintrin.h>

std::vector<std::tuple<int, int, int>>
getConflictPairsBitmap(PaceGraph &graph, SimpleLBParameter &parameter) {
    std::vector<std::tuple<int, int, int>> conflictPairs;

    std::vector<std::bitset<20000>> edges(graph.size_free);
    std::vector<std::bitset<20000>> notEdges(graph.size_free);

    for (int i = 0; i < graph.size_free; ++i) {
        for (int j = 0; j < graph.size_free; ++j) {
            if (graph.crossing.matrix_diff[i][j] < 0) {
                edges[i][j] = true;
            } else if (graph.crossing.matrix_diff[i][j] > 0) {
                notEdges[i][j] = true;
            }
        }
    }

    for (int u = 0; u < graph.size_free; u++) {
        auto &edgesU = edges[u];
        auto &notEdgesU = notEdges[u];

        for (int v = edgesU._Find_next(u); v < graph.size_free;
             v = edgesU._Find_next(v)) {
            auto &edgesV = edges[v];

            auto ws = notEdgesU & edgesV;
            for (int w = ws._Find_next(v); w < graph.size_free;
                 w = ws._Find_next(w)) {
                conflictPairs.emplace_back(u, v, w);
            }
        }

        if (conflictPairs.size() > parameter.maxNrOfConflicts) {
            break;
        }
    }

    return conflictPairs;
}

void fisherYatesShuffle(std::vector<std::tuple<int, int, int>> &conflictPairs,
                        std::mt19937 &rng) {
    for (int i = conflictPairs.size() - 1; i > (conflictPairs.size() / 2);
         --i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng);
        std::swap(conflictPairs[i], conflictPairs[j]);
    }
}

long improveWithPotential(PaceGraph &graph, SimpleLBParameter &parameter,
                          long currentLB) {
    auto conflictPairs = getConflictPairsBitmap(graph, parameter);

    if (conflictPairs.empty() ||
        conflictPairs.size() >= parameter.maxNrOfConflicts) {
        return 0;
    }

    long bestLBImprovement = 0;

    std::random_device rd;
    std::mt19937 rng(rd());
    for (int _ = 0; _ < parameter.numberOfIterationsForConflictOrder; _++) {
        if (parameter.nrOfConflictsToUsePseudoRandom < conflictPairs.size()) {
            if (_ % 2 == 0) {
                fisherYatesShuffle(conflictPairs, rng);
            } else {
                std::reverse(conflictPairs.begin(), conflictPairs.end());
            }
        } else {
            std::shuffle(conflictPairs.begin(), conflictPairs.end(), rng);
        }

        std::vector<std::vector<int>> potentialMatrix(
            graph.crossing.matrix_diff.size());

        for (int i = 0; i < graph.crossing.matrix_diff.size(); i++) {
            potentialMatrix[i].resize(graph.crossing.matrix_diff.size());
            for (int j = 0; j < graph.crossing.matrix_diff.size(); ++j) {
                potentialMatrix[i][j] = graph.crossing.matrix_diff[i][j];
            }
        }

        long currentLBImprovement = 0;

        for (auto [u, v, w] : conflictPairs) {
            if (potentialMatrix[u][v] < 0 && potentialMatrix[v][w] < 0 &&
                potentialMatrix[u][w] > 0) {

                int potential1 = -potentialMatrix[u][v];
                int potential2 = -potentialMatrix[v][w];
                int potential3 = potentialMatrix[u][w];

                if (parameter.testForceChoiceOfConflicts) {
                    long currentDiff =
                        graph.ub - currentLB - currentLBImprovement;

                    if (potential1 > currentDiff && !graph.crossing.lt(u, v)) {
                        graph.crossing.set_a_lt_b(u, v);
                    }

                    if (potential2 > currentDiff && !graph.crossing.lt(v, w)) {
                        graph.crossing.set_a_lt_b(v, w);
                    }

                    if (potential3 > currentDiff && !graph.crossing.lt(w, u)) {
                        graph.crossing.set_a_lt_b(w, u);
                    }
                }

                int minValue =
                    std::min(std::min(potential1, potential2), potential3);

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

long simpleLB(PaceGraph &graph, SimpleLBParameter &parameter) {
    bool canInitCrossingMatrix = graph.init_crossing_matrix_if_necessary();
    long lb = 0;

    if (!canInitCrossingMatrix) {
        for (int u = 0; u < graph.size_free; ++u) {
            for (int v = u + 1; v < graph.size_free; v++) {
                int crossing_matrix_u_v;
                int crossing_matrix_v_u;
                std::tie(crossing_matrix_u_v, crossing_matrix_v_u) =
                    graph.calculatingCrossingNumber(u, v);

                lb += std::min(crossing_matrix_u_v, crossing_matrix_v_u);
            }
        }
        return lb;
    }

    for (int u = 0; u < graph.size_free; ++u) {
        for (int v = u + 1; v < graph.size_free; v++) {
            lb += std::min(graph.crossing.matrix[u][v],
                           graph.crossing.matrix[v][u]);
        }
    }

    if (parameter.usePotentialMatrix) {
        lb += improveWithPotential(graph, parameter, lb);
    }

    if (graph.lb < lb) {
        graph.lb = lb;
    }

    return lb;
}