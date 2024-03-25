#ifndef ORDER_HPP
#define ORDER_HPP

// #include "pace_graph.hpp"

#include "pace_graph.hpp"
#include "relation.hpp"
#include "segment_tree.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>
#include <vector>

const int EXTREME_SWAPPING_COST = 10000;

class Order {
  private:
    // TODO(Lukas): These vectors should hold unsigned ints.
    std::vector<int> vertex_to_position;
    std::vector<int> position_to_vertex;

  public:
    PartialOrdering partial_order;
    /**
     * Creates an Order with a given size. The order will be the identity
     * permutation.
     */
    explicit Order(int size) {
        for (int i = 0; i < size; ++i) {
            vertex_to_position.push_back(i);
            position_to_vertex.push_back(i);
        }
    }

    /**
     * Creates an Order from a given position_to_vertex vector.
     * @param position_to_vertex_input for each position i, the vertex at that
     */
    explicit Order(const std::vector<int> &position_to_vertex_input)
        : position_to_vertex(position_to_vertex_input),
          vertex_to_position(position_to_vertex_input.size()) {

        for (size_t i = 0; i < position_to_vertex.size(); ++i) {
            int vertex = position_to_vertex[i];
            vertex_to_position[vertex] = i;
        }
    }

    void apply_reduction_rules() {}

    void swap_by_vertices(const int v, const int u) {
        int pos1 = vertex_to_position[v];
        int pos2 = vertex_to_position[u];

        vertex_to_position[v] = pos2;
        vertex_to_position[u] = pos1;

        position_to_vertex[pos1] = u;
        position_to_vertex[pos2] = v;
    }

    int get_position(const int vertex) { return vertex_to_position[vertex]; }
    int get_vertex(const int position) { return position_to_vertex[position]; }

    void swap_by_position(const int pos1, const int pos2) {
        int u = position_to_vertex[pos1];
        int v = position_to_vertex[pos2];
        swap_by_vertices(u, v);
    }

    void move_vertex(int vertex, int new_position) {
        int old_position = vertex_to_position[vertex];

        // Check if we need to shift vertices forward or backward.
        if (old_position < new_position) {
            // Move each vertex one position back, from old_position + 1 to
            // new_position.
            for (int i = old_position; i < new_position; ++i) {
                int next_vertex = position_to_vertex[i + 1];
                position_to_vertex[i] = next_vertex;
                vertex_to_position[next_vertex] = i;
            }
        } else if (old_position > new_position) {
            // Move each vertex one position forward, from old_position - 1 to
            // new_position.
            for (int i = old_position; i > new_position; --i) {
                int prev_vertex = position_to_vertex[i - 1];
                position_to_vertex[i] = prev_vertex;
                vertex_to_position[prev_vertex] = i;
            }
        }
        // Place the vertex at its new position.
        position_to_vertex[new_position] = vertex;
        vertex_to_position[vertex] = new_position;
    }

    /**
     * Returns the cost change if the vertices at position pos1 and pos2 are
     * swapped, but the swap will not be performed.
     * @return the cost change
     */
    int cost_change_if_swap_position(const int pos1, const int pos2,
                                     const PaceGraph &graph) {

        if (pos1 > pos2) {
            return cost_change_if_swap_position(pos2, pos1, graph);
        }

        int u = position_to_vertex[pos1];
        int v = position_to_vertex[pos2];
        // Vertices already commited to by reduction rules in the partial order
        // are supposed to be extremely costly to swap
        if (partial_order.lt(v, u)) {
            return EXTREME_SWAPPING_COST;
        }

        int cost_change = 0;
        // TODO: this can be AVX accelerated

        auto crossing_matrix_diff_u = graph.crossing_matrix_diff[u];

        auto crossing_matrix_diff_v = graph.crossing_matrix_diff[v];

        for (int pos = pos1; pos < pos2; pos++) {
            int w = position_to_vertex[pos];
            cost_change -= crossing_matrix_diff_u[w];
            cost_change += crossing_matrix_diff_v[w];
        }

        return cost_change;
    }

    long count_crossings(const PaceGraph &graph) {
        long crossings = 0;

        std::vector<int> seg_tree_array;
        seg_tree_array.assign(graph.size_free, 0);

        SegmentTree segTree(seg_tree_array);

        for (int i = 0; i < graph.size_fixed; i++) {

            std::vector<int> sorted_neighbors = graph.neighbors_fixed[i];
            std::sort(sorted_neighbors.begin(), sorted_neighbors.end(),
                      [&](int a, int b) {
                          return vertex_to_position[a] < vertex_to_position[b];
                      });

            for (int v : sorted_neighbors) {
                int posOfV = vertex_to_position[v];
                segTree.updateSingle(posOfV, 1);
                crossings += segTree.queryRange(posOfV + 1, graph.size_free);
            }
        }

        return crossings;
    }

    // Reduction rules from
    // https://www.sciencedirect.com/science/article/pii/S1570866707000469
    //
    // TOOD(Lukas): We need an upper bound for RRlarge rule -> Median Heuristic?
    // (No guarantees though)

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
    bool rr1_rr2(const PaceGraph &graph) {
        bool applied = false;
        for (int a = 0; a < graph.size_free; a++) {
            for (int b = a + 1; b < graph.size_free; b++) {
                // RR1
                if (graph.crossing_matrix[a][b] == 0) {
                    applied = partial_order.set_a_lt_b(a, b) || applied;
                } else if (graph.crossing_matrix[b][a] == 0) {
                    applied = partial_order.set_a_lt_b(b, a) || applied;
                } else if (graph.neighbors_free[a] == graph.neighbors_free[b]) {
                    // RR2
                    applied = partial_order.set_a_lt_b(a, b) || applied;
                }
            }
        }
        return applied;
    }

    /*
     * Applies reduction rule RR3 from
     * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
     * RR3: For every pair {a, b} with c_{a, b} = 2 and c{b, a} = 1 commit b < a
     * in partial_order.
     *
     * @return Boolean value indicating whether the reduction rule was applied.
     */
    // TODO: I am not convinced that this one can be applied in our case
    // bool rr3(const PaceGraph &graph) {
    //    bool applied = false;
    //    for (int a = 0; a < graph.size_free; a++) {
    //        for (int b = 0; b < graph.size_free; b++) {
    //            if (graph.crossing_matrix[a][b] == 2 &&
    //                graph.crossing_matrix[b][a] == 1) {
    //                applied = partial_order.set_a_lt_b(a, b) || applied;
    //            }
    //        }
    //    }
    //    return applied;
    //}

    /*
     * Applies reduction rule RRLarge from
     * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
     * RRLarge: For every pair {a, b} with c_{b, a} > upper_bound commit a < b
     * in partial_order.
     *
     * @return Boolean value indicating whether the reduction rule was applied.
     */
    bool rrlarge(const PaceGraph &graph, int upper_bound) {
        bool applied = false;
        for (int a = 0; a < graph.size_free; a++) {
            for (int b = a + 1; a < graph.size_free; b++) {
                if (graph.crossing_matrix_diff[a][b] > upper_bound) {
                    applied = partial_order.set_a_lt_b(b, a) || applied;
                } else if (graph.crossing_matrix_diff[b][a] > upper_bound) {
                    applied = partial_order.set_a_lt_b(a, b) || applied;
                }
            }
        }
        return applied;
    }

    /*
     * Applies reduction rules RRL01 and RRL02 from
     * https://www.sciencedirect.com/science/article/pii/S1570866707000469.
     * RRLO1: For every v that is comparable to all other vertices in the order
     * we delete v from the graph
     * RRLO2: For every pair {v, w} that is not dependent in partial_order we
     * commit v < w in partial_order
     * @return Boolean value indicating whether the reduction rules were
     * applied.
     */
    bool rrlo1_rrlo2(PaceGraph &graph) {
        bool applied = false;
        std::unordered_set<int> vertices_to_delete;
        for (int v = 0; v < graph.size_free; v++) {
            for (int w = 0; w < graph.size_free; w++) {
                if (partial_order.incomparable(v, w)) {
                    // RRLO2
                    for (int c = 0; c < graph.size_free; c++) {
                        if (partial_order.dependent(v, w, c)) {
                            goto keep_v;
                        }
                    }
                    // (v,w) is not dependent
                    if (graph.crossing_matrix[v][w] <=
                        graph.crossing_matrix[w][v]) {
                        applied = partial_order.set_a_lt_b(v, w) || applied;
                    }
                    goto keep_v;
                }
            }
            // RRLO1 -> v is comparable to all elements in the partial order
            applied = true;
            vertices_to_delete.insert(v);
        keep_v:;
        }

        // TODO: Depending on performance and how often we remove vertices we
        // could batch delete them.
        for (int vertex : vertices_to_delete) {
            graph.remove_free_vertex(vertex);
        }
        return applied;
    }

    /**
     * Randomly permutes the order.
     */
    void permute() {
        std::random_device rd;
        std::mt19937 g(rd());

        // Shuffle the position_to_vertex vector
        std::shuffle(position_to_vertex.begin(), position_to_vertex.end(), g);

        // Update the vertex_to_position vector to reflect the new positions
        for (size_t i = 0; i < position_to_vertex.size(); ++i) {
            vertex_to_position[position_to_vertex[i]] = i;
        }
    }

    std::string convert_to_real_node_id(PaceGraph &graph) {
        std::ostringstream result;
        for (const auto &vertex : position_to_vertex) {
            result << graph.free_real_names[vertex] << "\n";
        }

        return result.str();
    }
    Order clone() { return Order(std::vector(position_to_vertex)); }

    PaceGraph reorderGraph(PaceGraph &graph) {
        std::vector<std::tuple<int, int>> new_edges;
        std::unordered_map<int, int> new_free_real_names;

        for (int oldV = 0; oldV < graph.size_free; ++oldV) {
            int newV = get_position(oldV);
            new_free_real_names[newV] = graph.free_real_names[oldV];

            for (int u : graph.neighbors_free[oldV]) {
                new_edges.emplace_back(u, newV);
            }
        }

        return {graph.size_fixed, graph.size_free, new_edges,
                graph.fixed_real_names, new_free_real_names};
    }
};

#endif // ORDER_HPP
