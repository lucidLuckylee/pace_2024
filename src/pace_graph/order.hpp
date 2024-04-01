#ifndef ORDER_HPP
#define ORDER_HPP

// #include "pace_graph.hpp"

#include "pace_graph.hpp"
#include "segment_tree.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

class Order {
  private:
    // TODO(Lukas): These vectors should hold unsigned ints.
    std::vector<int> vertex_to_position;
    std::vector<int> position_to_vertex;

  public:
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
