#include "mean_position_heuristic.hpp"

Order MeanPositionSolver::jittering(PaceGraph &graph, int iteration) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1, 1);

    Order currentBestOrder = Order(graph.size_free);
    long bestOrderCost = 1000000000000000000;

    std::vector<double> nodeOffset = std::vector<double>(graph.size_free);
    std::vector<std::tuple<int, double>> average_position(graph.size_free);
    for (int i = 0; i < graph.size_free; ++i) {
        if (meanPositionParameter.useJittering) {
            nodeOffset[i] = dis(gen);
        } else {
            nodeOffset[i] = 0;
        }
    }

    for (int _ = 0; _ < meanPositionParameter.jitterIterations; ++_) {
        std::vector<double> newNodeOffset = std::vector<double>(nodeOffset);
        for (int j = 0; j < graph.size_free; ++j) {
            newNodeOffset[j] += dis(gen) / 10;
        }
        for (int i = 0; i < graph.neighbors_free.size(); ++i) {
            auto neighbors_of_node = graph.neighbors_free[i];

            double avg = 0;

            switch (meanPositionParameter.meanType) {
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
                auto crossing_matrix_for_i = graph.crossing.matrix[i];
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

        if (!meanPositionParameter.useJittering || !has_time_left(iteration)) {
            break;
        }
    }

    return currentBestOrder;
}

void MeanPositionSolver::improveOrderWithSwapping(PaceGraph &graph,
                                                  Order &order, int iteration) {
    bool foundSwap = true;

    while (foundSwap && has_time_left(iteration)) {
        foundSwap = false;

        for (int i = 0; i < graph.size_free - 1; ++i) {
            if (!has_time_left(iteration)) {
                break;
            }
            int u = order.get_vertex(i);
            int v = order.get_vertex(i + 1);

            int crossing_matrix_u_v;
            int crossing_matrix_v_u;
            std::tie(crossing_matrix_u_v, crossing_matrix_v_u) =
                graph.calculatingCrossingNumber(u, v);

            if (crossing_matrix_u_v > crossing_matrix_v_u) {
                order.swap_by_position(i, i + 1);
                foundSwap = true;
            }
        }
    }
}

Order MeanPositionSolver::solve(PaceGraph &graph) {

    Order currentBestOrder = Order(graph.size_free);
    long bestOrderCost = 1000000000000000000;

    int iteration = 0;
    while (has_time_left(iteration)) {
        Order order = jittering(graph, iteration);
        improveOrderWithSwapping(graph, order, iteration);

        if (!has_time_left(iteration)) {
            if (bestOrderCost == 1000000000000000000) {
                return order;
            }
            break;
        }

        long cost = order.count_crossings(graph);
        if (cost <= bestOrderCost) {
            bestOrderCost = cost;
            currentBestOrder = order;
        }
        iteration++;
    }

    // std::cout << "#Iterations: " << iteration << std::endl;

    return currentBestOrder;
}
