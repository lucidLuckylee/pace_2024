//
// Created by jt on 10.03.24.
//

#include "simple_lb.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <x86gprintrin.h>

std::vector<std::tuple<int, int, int>>
getConflictPairsIterateOverMatrix(PaceGraph &graph) {
    std::vector<std::tuple<int, int, int>> conflictPairs;

    for (int u = 0; u < graph.size_free; ++u) {
        for (int v = 0; v < graph.size_free; ++v) {
            if (u == v)
                continue;
            if (graph.crossing_matrix_diff[u][v] < 0) {
                auto vDiffs = graph.crossing_matrix_diff[v];
                auto uDiffs = graph.crossing_matrix_diff[u];
                for (int w = 0; w < graph.size_free; w++) {
                    if (w == u || w == v)
                        continue;

                    if (vDiffs[w] < 0 && uDiffs[w] > 0) {
                        conflictPairs.emplace_back(u, v, w);
                    }
                }
            }
        }
    }

    return conflictPairs;
}

std::vector<std::tuple<int, int, int>>
getConflictPairsIterateOverNeighbors(PaceGraph &graph) {
    std::vector<std::tuple<int, int, int>> conflictPairs;

    std::vector<std::vector<int>> smaller(graph.size_free);

    for (int i = 0; i < graph.size_free; ++i) {
        smaller[i] = std::vector<int>();
        for (int j = 0; j < graph.size_free; ++j) {
            if (i == j)
                continue;
            if (graph.crossing_matrix_diff[i][j] < 0) {
                smaller[i].push_back(j);
            }
        }
    }

    for (int u = 0; u < graph.size_free; ++u) {
        for (int v : smaller[u]) {
            for (int w : smaller[v]) {
                if (graph.crossing_matrix_diff[u][w] > 0) {
                    conflictPairs.emplace_back(u, v, w);
                }
            }
        }
    }

    return conflictPairs;
}

std::vector<std::tuple<int, int, int>>
getConflictPairsBMI(PaceGraph &graph, SimpleLBParameter &parameter) {
    std::vector<std::tuple<int, int, int>> conflictPairs;

    std::vector<std::vector<uint64_t>> smaller(graph.size_free);
    std::vector<std::vector<uint64_t>> larger(graph.size_free);

    for (int i = 0; i < graph.size_free; ++i) {
        smaller[i] = std::vector<uint64_t>((graph.size_free + 63) / 64);
        larger[i] = std::vector<uint64_t>((graph.size_free + 63) / 64);
        for (int j = 0; j < graph.size_free; ++j) {
            int pos = j / 64;
            int bit = j % 64;
            if (graph.crossing_matrix_diff[i][j] < 0) {
                smaller[i][pos] |= (1L << bit);
            } else if (graph.crossing_matrix_diff[i][j] > 0) {
                larger[i][pos] |= (1L << bit);
            }
        }
    }

    for (int u = 0; u < graph.size_free; u++) {
        auto smaller_u = smaller[u];

        for (int pos1 = 0; pos1 < smaller_u.size(); pos1++) {
            auto vs = smaller_u[pos1];
            while (vs) {
                int bit = __builtin_ctzll(vs);
                vs = __blsr_u64(vs);

                int v = pos1 * 64 + bit;

                for (int pos2 = 0; pos2 < smaller_u.size(); pos2++) {
                    auto ws1 = smaller[v][pos2];
                    auto ws2 = larger[u][pos2];

                    auto ws = ws1 & ws2;
                    while (ws) {
                        int bit2 = __builtin_ctzll(ws);
                        ws = __blsr_u64(ws);

                        int w = pos2 * 64 + bit2;
                        conflictPairs.emplace_back(u, v, w);
                    }
                }
            }
        }
 
        if (conflictPairs.size() > parameter.maxNrOfConflicts) {
            break;
        }
    }

    return conflictPairs;
}

std::vector<std::tuple<int, int, int>>
getConflictPairs(SimpleLBParameter &parameter, PaceGraph &graph) {
    std::vector<std::tuple<int, int, int>> conflictPairs;
    if (parameter.searchStrategyForConflicts ==
        SearchStrategyForConflicts::MATRIX) {
        conflictPairs = getConflictPairsIterateOverMatrix(graph);
    } else if (parameter.searchStrategyForConflicts ==
               SearchStrategyForConflicts::NEIGHBORS) {
        conflictPairs = getConflictPairsIterateOverNeighbors(graph);
    } else if (parameter.searchStrategyForConflicts ==
               SearchStrategyForConflicts::BMI) {
        conflictPairs = getConflictPairsBMI(graph, parameter);
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
    auto conflictPairs = getConflictPairs(parameter, graph);

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
            graph.crossing_matrix_diff.size());

        for (int i = 0; i < graph.crossing_matrix_diff.size(); i++) {
            potentialMatrix[i].resize(graph.crossing_matrix_diff.size());
            for (int j = 0; j < graph.crossing_matrix_diff.size(); ++j) {
                potentialMatrix[i][j] = graph.crossing_matrix_diff[i][j];
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

                    if (potential1 > currentDiff &&
                        graph.crossing_matrix[v][u] < INF) {
                        graph.fixNodeOrder(u, v);
                    }

                    if (potential2 > currentDiff &&
                        graph.crossing_matrix[v][w] < INF) {
                        graph.fixNodeOrder(v, w);
                    }

                    if (potential3 > currentDiff &&
                        graph.crossing_matrix[u][w] < INF) {
                        graph.fixNodeOrder(w, u);
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
                int crossing_matrix_u_v = 0;
                int crossing_matrix_v_u = 0;

                for (int u_N : graph.neighbors_free[u]) {
                    for (int v_N : graph.neighbors_free[v]) {
                        if (u_N > v_N) {
                            crossing_matrix_u_v++;
                        } else if (v_N > u_N) {
                            crossing_matrix_v_u++;
                        }
                    }
                }

                lb += std::min(crossing_matrix_u_v, crossing_matrix_v_u);
            }
        }
        return lb;
    }

    for (int u = 0; u < graph.size_free; ++u) {
        for (int v = u + 1; v < graph.size_free; v++) {
            lb += std::min(graph.crossing_matrix[u][v],
                           graph.crossing_matrix[v][u]);
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