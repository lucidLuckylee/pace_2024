
#include "data_reduction_rules.hpp"
#include "../lb/simple_lb.hpp"
#include "../pace_graph/directed_graph.hpp"

/*
 * Applies reduction rule RR1 from
 * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
 * RR1: For every pair {a, b} with c_{a,b} = 0 commit a < b in
 * partial_order.
 */
void rr1(PaceGraph &graph) {
    for (int a = 0; a < graph.size_free; a++) {
        for (int b = a + 1; b < graph.size_free; b++) {
            // RR1
            if (graph.crossing.matrix[a][b] == 0) {
                graph.crossing.set_a_lt_b(a, b);
            } else if (graph.crossing.matrix[b][a] == 0) {
                graph.crossing.set_a_lt_b(b, a);
            }
        }
    }
}

/*
 * Applies reduction rule RR2 from
 * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
 * RR2: For every pair {a, b} with N(a) == N(b) arbitrarily
 * commit a < b in partial_order.
 *
 */
void rr2(PaceGraph &graph) {
    for (int a = 0; a < graph.size_free; a++) {
        for (int b = a + 1; b < graph.size_free; b++) {
            if (graph.neighbors_free[a] == graph.neighbors_free[b]) {
                graph.crossing.set_a_lt_b(a, b);
            }
        }
    }
}

/*void rr2(PaceGraph &graph) {
    for (int a = 0; a < graph.size_free; a++) {
        for (int b = a + 1; b < graph.size_free; b++) {
            if (graph.neighbors_free[a].size() > 0 &&
                graph.neighbors_free[b].size() > 0) {
                if (graph.neighbors_free[a][0] == graph.neighbors_free[b][0] &&
                    graph.neighbors_free[a].back() ==
                        graph.neighbors_free[b].back()) {
                    if (graph.crossing.matrix[a][b] <=
                        graph.crossing.matrix[b][a]) {
                        graph.crossing.set_a_lt_b(a, b);
                    } else {
                        graph.crossing.set_a_lt_b(b, a);
                    }
                }
            }
        }
    }
}*/

void rr3(PaceGraph &graph) {
    for (int a = 0; a < graph.size_free; a++) {
        for (int b = a + 1; b < graph.size_free; b++) {
            // RR1
            if (graph.crossing.matrix[a][b] == 2 &&
                graph.crossing.matrix[b][a] == 1) {
                // Check if we are in case 1 of Lemma 3
                if (graph.neighbors_free[a].size() == 2 &&
                    graph.neighbors_free[b].size() == 2) {
                    graph.crossing.set_a_lt_b(a, b);
                }
            } else if (graph.crossing.matrix[a][b] == 1 &&
                       graph.crossing.matrix[b][a] == 2) {
                if (graph.neighbors_free[a].size() == 2 &&
                    graph.neighbors_free[b].size() == 2) {
                    graph.crossing.set_a_lt_b(b, a);
                }
            }
        }
    }
}

/*
 * Applies reduction rule RRLarge (MODIFIED) from
 * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
 * RRLarge:  Let lb be the simple lb. For every pair {a, b} with c_{b, a} >
 * upper_bound - lb commit a < b in partial_order.
 *
 */
void rrlarge(PaceGraph &graph) {
    long ub = graph.ub;
    SimpleLBParameter parameter;
    parameter.usePotentialMatrix = false;
    long lb = simpleLB(graph, parameter);

    for (int a = 0; a < graph.size_free; a++) {
        for (int b = a + 1; b < graph.size_free; b++) {
            if (graph.crossing.matrix_diff[a][b] > ub - lb) {
                graph.crossing.set_a_lt_b(b, a);
            } else if (graph.crossing.matrix_diff[b][a] > ub - lb) {
                graph.crossing.set_a_lt_b(a, b);
            }
        }
    }
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
    std::vector<DeleteInfo> vertices_to_delete;
    std::vector<bool> already_deleted(graph.size_free, false);

    for (int v = 0; v < graph.size_free; v++) {
        int position = 0;
        bool delete_v = true;
        long cost = 0;
        for (int w = 0; w < graph.size_free; w++) {
            if (graph.crossing.comparable(v, w)) {
                if (graph.crossing.lt(w, v)) {
                    position++;
                }
                if (already_deleted[w]) {
                    continue;
                }
                if (graph.crossing.lt(w, v)) {
                    cost += graph.crossing.matrix[w][v];
                } else {
                    cost += graph.crossing.matrix[v][w];
                }
            } else {
                delete_v = false;
                break;
            }
        }
        if (delete_v) {
            // RRLO1 -> v is comparable to all elements in the partial order
            already_deleted[v] = true;
            vertices_to_delete.emplace_back(v, position, cost);
        }
    }

    if (vertices_to_delete.empty()) {
        return false;
    }
    graph.remove_free_vertices(vertices_to_delete);
    return true;
}

bool rrlo2(PaceGraph &graph) {
    bool applied = false;
    for (int v = 0; v < graph.size_free; v++) {
        for (int w = 0; w < graph.size_free; w++) {
            if (graph.crossing.incomparable(v, w)) {
                for (int c = 0; c < graph.size_free; c++) {
                    if (graph.crossing.dependent(v, w, c)) {
                        goto next_w;
                    }
                }
                // (v,w) is not dependent
                if (graph.crossing.matrix[v][w] <=
                    graph.crossing.matrix[w][v]) {
                    applied = graph.crossing.set_a_lt_b(v, w) || applied;
                }
            }
        next_w:;
        }
    }
    return applied;
}

void rrtransitive(PaceGraph &graph) {
    auto dg = DirectedGraph::dag_from_partial_order(graph.crossing);
    dg.init_reachability_matrix_dag();
    for (int i = 0; i < graph.size_free; i++) {
        for (int j = 0; j < graph.size_free; j++) {
            if (dg.reachabilityMatrix[i][j]) {
                graph.crossing.set_a_lt_b(i, j);
            }
        }
    }
}

void apply_reduction_rules(PaceGraph &graph) {
    if (!graph.init_crossing_matrix_if_necessary()) {
        return;
    }

    rr1(graph);
    rr2(graph);
    // rr3(graph);
    rrlarge(graph);
    rrtransitive(graph);
    rrlo1(graph);
    // Uncomment this to re-enable rrlo2.
    // while (rrlo2(graph)) {
    //     rrtransitive(graph);
    //     while (rrlo1(graph)) {
    //         rrlarge(graph);
    //         rrtransitive(graph);
    //     };
    // };
}
