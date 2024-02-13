#include "cheap_heuristics.h"

Order mean_position_algorithm(PaceGraph &graph, MeanTypeAlgo meanType) {
    std::vector<std::tuple<int, double>> average_position;

    for (int i = 0; i < graph.neighbors_free.size(); ++i) {
        auto neighbors_of_node = graph.neighbors_free[i];
        double avg = 0;

        switch (meanType) {
        case average:
            for (auto neighbor : neighbors_of_node) {
                avg += neighbor;
            }
            avg /= neighbors_of_node.size();
            break;
        case median:
            if (neighbors_of_node.size() != 0) {
                int position_middle_node = neighbors_of_node.size() / 2;
                avg = neighbors_of_node[position_middle_node];
            }
            break;
        }

        average_position.emplace_back(i, avg);
    }
    std::sort(
        average_position.begin(), average_position.end(),
        [](const std::tuple<int, double> &a, const std::tuple<int, double> &b) {
            return std::get<1>(a) <
                   std::get<1>(b); // Compare based on the second element
        });
    std::vector<int> order;

    order.reserve(average_position.size());
    for (auto node_with_postion : average_position) {
        order.emplace_back(std::get<0>(node_with_postion));
    }
    return Order(order);
}