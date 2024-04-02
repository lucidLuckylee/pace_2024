
#include "../lb/simple_lb.hpp"
#include "../pace_graph/directed_graph.hpp"
#include <iostream>

/*
 * Applies reduction rules RR1 and RR2 from
 * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
 * RR1: For every pair {a, b} with c_{a,b} = 0 commit a < b in
 * partial_order.
 * RR2: For every pair {a, b} with N(a) == N(b) arbitrarily
 * commit a < b in partial_order.
 *
 * @return Boolean value indicating whether reduction rules were applied.
 */
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

/*
 * Applies reduction rule RRLarge (MODIFIED) from
 * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
 * RRLarge:  Let lb be the simple lb. For every pair {a, b} with c_{b, a} >
 * upper_bound - lb commit a < b in partial_order.
 *
 * @return Boolean value indicating whether the reduction rule was applied.
 */
bool rrlarge(PaceGraph &graph) {
    long ub = graph.ub;
    SimpleLBParameter parameter;
    parameter.usePotentialMatrix = false;
    long lb = simpleLB(graph, parameter);

    bool applied = false;
    for (int a = 0; a < graph.size_free; a++) {
        for (int b = a + 1; b < graph.size_free; b++) {
            if (graph.crossing.matrix_diff[a][b] > ub - lb) {
                applied = graph.crossing.set_a_lt_b(b, a) || applied;
            } else if (graph.crossing.matrix_diff[b][a] > ub - lb) {
                applied = graph.crossing.set_a_lt_b(a, b) || applied;
            }
        }
    }
    return applied;
}

/*
 * Applies reduction rules RRL01 from
 * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
 * RRLO1: For every v that is comparable to all other vertices in the order
 * we delete v from the graph
 * @return Boolean value indicating whether the reduction rules were
 * applied.
 */
bool rrlo1(PaceGraph &graph) {
    std::vector<std::tuple<int, int, int>> vertices_to_delete;

    std::vector<bool> already_deleted(graph.size_free, false);

    for (int v = 0; v < graph.size_free; v++) {
        int posOfV = 0;
        bool canDeleted = true;
        long costs = 0;
        for (int w = 0; w < graph.size_free; w++) {
            if (already_deleted[w]) {
                continue;
            }
            if (graph.crossing.comparable(v, w)) {
                if (graph.crossing.lt(w, v)) {
                    posOfV++;
                    costs += graph.crossing.matrix[w][v];
                } else {
                    costs += graph.crossing.matrix[v][w];
                }

            } else {
                canDeleted = false;
                break;
            }
        }

        if (canDeleted) {
            already_deleted[v] = true;
            // RRLO1 -> v is comparable to all elements in the partial order
            vertices_to_delete.emplace_back(v, posOfV, costs);
        }
    }

    if (vertices_to_delete.empty()) {
        return false;
    }

    graph.remove_free_vertices(vertices_to_delete);

    return true;
}
bool rrtransitive(PaceGraph &graph) {

    bool applied = false;
    std::vector<std::vector<int>> adjList(graph.size_free);
    for (int i = 0; i < graph.size_free; i++) {
        for (int j = 0; j < graph.size_free; j++) {
            if (graph.crossing.lt(i, j)) {
                adjList[i].push_back(j);
            }
        }
    }

    DirectedGraph directedGraph(adjList);
    directedGraph.init_reachability_matrix_dag();
    for (int i = 0; i < graph.size_free; i++) {
        for (int j = 0; j < graph.size_free; j++) {
            if (i != j && directedGraph.reachabilityMatrix[i][j]) {
                applied |= graph.crossing.set_a_lt_b(i, j);
            }
        }
    }

    return applied;
}
void apply_reduction_rules(PaceGraph &graph) {
    if (!graph.init_crossing_matrix_if_necessary()) {
        return;
    }
    int k = 0;
    while (rr1_rr2(graph)) {
        k += 1;
    }

    rrlarge(graph);

    rrtransitive(graph);
    rrlo1(graph);
}
