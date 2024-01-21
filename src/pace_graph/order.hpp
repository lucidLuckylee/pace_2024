#ifndef ORDER_HPP
#define ORDER_HPP

// #include "pace_graph.hpp"

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
template <typename Container> class Order {
  public:
    Container vertices;

    template <typename InputContainer> Order(const InputContainer &vertices) {
        for (const auto &vertex : vertices) {
            this->vertices.push_back(vertex);
        }
    }

    /*
     * @brief Create an Order from a range of integers.
     *
     * @param start included in the order
     * @param end not included in the order
     */
    Order(const int start, const int end) {
        vertices.resize(end);
        std::iota(vertices.begin(), vertices.end(), start);
    };

    /*
     * @brief Copy constructor.
     *
     * @param other Order to be copied
     */
    template <typename OtherContainer>
    Order(const Order<OtherContainer> &other) {
        for (const auto &vertex : other.vertices) {
            vertices.push_back(vertex);
        }
    };

    void swap_vertices(const int index_a, const int index_b) {
        auto it_a = std::next(std::begin(vertices), index_a);
        auto it_b = std::next(std::begin(vertices), index_b);

        std::iter_swap(it_a, it_b);
    }

    std::string to_string() {
        std::ostringstream result;
        for (const auto &vertex : vertices) {
            result << vertex << " ";
        }

        return result.str();
    }

    // TODO (Lukas, Fanny): Keep this here or move it to PaceGraph as
    //                      void count_crossings(Order order);
    //                      or somewhere else as
    //                      void count_crossings(Order order, PaceGraph graph);
    // TODO (Lukas): Implement me!
    // void count_crossings(const PaceGraph graph);

    // TODO (Lukas): Implement me!
    // void permute(int seed);
};

#endif // ORDER_HPP
