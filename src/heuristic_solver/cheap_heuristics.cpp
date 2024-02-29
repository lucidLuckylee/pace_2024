#include "cheap_heuristics.hpp"
#include "../pace_graph/solver.hpp"

MeanPositionSolver::MeanPositionSolver(PaceGraph &graph, MeanTypeAlgo meanType)
    : Solver(graph) {
    this->meanType = meanType;
}

Order MeanPositionSolver::terminate() {
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

Order MeanPositionSolver::solve() {
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
            this->graph->init_crossing_matrix_if_necessary();
            auto crossing_matrix_for_i = this->graph->crossing_matrix[i];
            for (int j = 0; j < this->graph->size_free; ++j) {
                avg += crossing_matrix_for_i[j];
            }
        }
        this->average_position.emplace_back(i, avg);
    }

    std::sort(
        this->average_position.begin(), this->average_position.end(),
        [](const std::tuple<int, double> &a, const std::tuple<int, double> &b) {
            // Compare based on the second element
            return std::get<1>(a) < std::get<1>(b);
        });

    return this->terminate();
}
