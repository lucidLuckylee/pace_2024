#ifndef ORDER_HPP
#define ORDER_HPP

// #include "pace_graph.hpp"

#include "pace_graph.hpp"
#include <iterator>
#include <numeric>
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
    explicit Order(int size) {
        for (int i = 0; i < size; ++i) {
            vertex_to_position.push_back(i);
            position_to_vertex.push_back(i);
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

    void swap_by_position(const int pos1, const int pos2) {
        int u = position_to_vertex[pos1];
        int v = position_to_vertex[pos2];
        swap_by_vertices(u, v);
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

                c += graph.crossing_matrix[u * position_to_vertex.size() + v];
            }
        }

        return c;
    }

    // TODO (Lukas, Fanny): Keep this here or move it to PaceGraph as
    //                      void count_crossings(Order order);
    //                      or somewhere else as
    //                      void count_crossings(Order order, PaceGraph graph);
    // TODO (Lukas): Implement me!

    // TODO (Lukas): Implement me!
    // void permute(int seed);
};

#endif // ORDER_HPP
