//
// Created by Lotte on 2024-02-13.
//

#ifndef PACE2024_CHEAP_HEURISTICS_H
#define PACE2024_CHEAP_HEURISTICS_H
#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"

enum MeanTypeAlgo { average, median, sum_along_crossing };

class MeanPositionSolver : Solver {
  public:
    MeanTypeAlgo meanType;
    std::vector<std::tuple<int, double>> average_position;

    explicit MeanPositionSolver(PaceGraph &graph, MeanTypeAlgo meanType)
        : Solver(graph) {
        this->meanType = meanType;
    }

    /*  Terminate the current solve function.
     *  Implement the generation of the resulting Order here
     *  so the signal_handler can call it.
     *  For this the Order has to be created from Class member variables.
     */
    Order terminate() {
        // The solver is about to return a result so we ignore any SIGTERM
        // signal now.
        signal(SIGTERM, SIG_IGN);
        std::vector<int> order;

        order.reserve(this->average_position.size());
        for (auto node_with_postion : this->average_position) {
            order.emplace_back(std::get<0>(node_with_postion));
        }
        return Order(order);
    }

    Order solve() {
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
            this->average_position.emplace_back(i, avg);
        }
        
        std::sort(this->average_position.begin(), this->average_position.end(),
                  [](const std::tuple<int, double> &a,
                     const std::tuple<int, double> &b) {
                      // Compare based on the second element
                      return std::get<1>(a) < std::get<1>(b);
                  });

        return this->terminate();
    }
};

#endif // PACE2024_CHEAP_HEURISTICS_H
