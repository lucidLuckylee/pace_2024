//
// Created by Lotte on 2024-02-13.
//

#ifndef PACE2024_CHEAP_HEURISTICS_H
#define PACE2024_CHEAP_HEURISTICS_H
#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"
#include <iostream>

enum MeanTypeAlgo { average, median, sum_along_crossing };

class MeanPositionSolver : Solver {
  public:
    MeanTypeAlgo meanType;

    explicit MeanPositionSolver(PaceGraph& graph, MeanTypeAlgo meanType) : Solver(graph){
        this->meanType = meanType;
    }

    Order terminate() {
        // TODO: Figure out a way to infer an Order from the current state
        return Order(6);
    }

    Order solve() {
        std::vector<std::tuple<int, double>> average_position;
        for (int i = 0; i < this->graph->neighbors_free.size(); ++i) {
            auto neighbors_of_node = this->graph->neighbors_free[i];
            double avg = 0;
            switch (this->meanType) {
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
            case sum_along_crossing:
                auto crossing_matrix_for_i = this->graph->crossing_matrix[i];
                for (int j = 0; j < this->graph->size_free; ++j) {
                    avg += crossing_matrix_for_i[j];
                }
            }

            average_position.emplace_back(i, avg);
        }
        std::sort(average_position.begin(), average_position.end(),
                  [](const std::tuple<int, double> &a,
                     const std::tuple<int, double> &b) {
                      return std::get<1>(a) <
                             std::get<1>(
                                 b); // Compare based on the second element
                  });
        std::vector<int> order;

        order.reserve(average_position.size());
        for (auto node_with_postion : average_position) {
            order.emplace_back(std::get<0>(node_with_postion));
        }
        return Order(order);
    }
};

#endif // PACE2024_CHEAP_HEURISTICS_H
