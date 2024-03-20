#include "cheap_heuristics.hpp"

MeanPositionSolver::MeanPositionSolver(
    CheapHeuristicsParameter cheapHeuristicsParameter) {
    this->cheapHeuristicsParameter = cheapHeuristicsParameter;
}

Order MeanPositionSolver::solve(PaceGraph &graph) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1, 1);

    Order currentBestOrder = Order(graph.size_free);
    long bestOrderCost = 1000000000000000000;

    std::vector<double> nodeOffset = std::vector<double>(graph.size_free);
    std::vector<std::tuple<int, double>> average_position(graph.size_free);
    for (int i = 0; i < graph.size_free; ++i) {
        nodeOffset[i] = dis(gen);
    }
 
    for (int _ = 0; _ < cheapHeuristicsParameter.jitterIterations; ++_) {
        std::vector<double> newNodeOffset = std::vector<double>(nodeOffset);
        for (int j = 0; j < graph.size_free; ++j) {
            newNodeOffset[j] += dis(gen) / 10;
        }
        for (int i = 0; i < graph.neighbors_free.size(); ++i) {
            auto neighbors_of_node = graph.neighbors_free[i];

            double avg = 0;

            switch (cheapHeuristicsParameter.meanType) {
            case average:
                for (auto neighbor : neighbors_of_node) {
                    avg += neighbor;
                }
                avg /= neighbors_of_node.size();
                break;
            case median:
                if (neighbors_of_node.size() != 0) {
                    int position_middle_node = neighbors_of_node.size() / 2;
                    avg = neighbors_of_node[position_middle_node] +
                          newNodeOffset[i];
                }
                break;
            case sum_along_crossing:
                graph.init_crossing_matrix_if_necessary();
                auto crossing_matrix_for_i = graph.crossing_matrix[i];
                for (int j = 0; j < graph.size_free; ++j) {
                    avg += crossing_matrix_for_i[j];
                }
            }
            average_position[i] = std::tuple(i, avg);
        }

        std::sort(average_position.begin(), average_position.end(),
                  [](const std::tuple<int, double> &a,
                     const std::tuple<int, double> &b) {
                      // Compare based on the second element
                      return std::get<1>(a) < std::get<1>(b);
                  });

        std::vector<int> orderVector(graph.size_free);
        for (int i = 0; i < graph.size_free; ++i) {
            orderVector[i] = std::get<0>(average_position[i]);
        }

        Order order = Order(orderVector);
        long cost = order.count_crossings(graph);
        if (cost <= bestOrderCost) {
            bestOrderCost = cost;
            currentBestOrder = order;
            nodeOffset = newNodeOffset;
        }
    }

    return currentBestOrder;
}
