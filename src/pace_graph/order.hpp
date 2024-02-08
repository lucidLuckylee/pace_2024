#ifndef ORDER_HPP
#define ORDER_HPP

// #include "pace_graph.hpp"

#include "pace_graph.hpp"
#include <algorithm>
#include <iterator>
#include <numeric>
#include <random>
#include <sstream>
#include <vector>

/*
 *  This class makes use of templating to be able to swap out the
 *  Container implementation used by it to any that supports iter_swap.
 *  Thus, you can e.g. create an Order with a std::vector or std::list
 *  as Container.
 *
 *  Example:
 *      Order<std::list<int>> test_order(1,4);
 *      Order<std::deque<int>> test_order_2(test_order);
 *      Order<std::vector<int>> test_order_3(std::vector({1,2,3,4}));
 *
 */
class Order {
  private:
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

        int cost_change = 0;
        // TODO: this can be AVX accelerated (we may have to save the
        // crossing_matrix two times (once transposed))
        for (int pos = pos1; pos < pos2; pos++) {
            int w = position_to_vertex[pos];
            cost_change -= graph.crossing_matrix[u][w];
            cost_change += graph.crossing_matrix[w][u];

            cost_change += graph.crossing_matrix[v][w];
            cost_change -= graph.crossing_matrix[w][v];
        }

        return cost_change;
    }

    std::string to_string() {
        std::ostringstream result;
        for (const auto &vertex : position_to_vertex) {
            result << vertex << "\n";
        }

        return result.str();
    }

    int count_crossings(const PaceGraph graph) {
        int c = 0;
        for (int i = 0; i < position_to_vertex.size(); i++) {
            for (int j = i + 1; j < position_to_vertex.size(); j++) {
                int u = position_to_vertex[i];
                int v = position_to_vertex[j];

                c += graph.crossing_matrix[u][v];
            }
        }

        return c;
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
};

#endif // ORDER_HPP
