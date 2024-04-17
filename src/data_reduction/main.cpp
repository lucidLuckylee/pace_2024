#include "../data_reduction/data_reduction_rules.hpp"
#include "../heuristic_solver/greedy_insert_solver.h"
#include "../pace_graph/pace_graph.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);

    std::tuple<std::vector<std::unique_ptr<PaceGraph>>, std::vector<int>> val =
        graph.splitGraphs();
    std::vector<std::unique_ptr<PaceGraph>> splittedGraphs =
        std::move(std::get<0>(val));
    std::vector<int> isolated_nodes = std::move(std::get<1>(val));
    //std::cerr << "Graph was split into: " << splittedGraphs.size() << std::endl;
    for (int i = 0; i < splittedGraphs.size(); i++) {
        auto &subgraph = splittedGraphs[i];
        //std::cerr << "Instance " << i << " size: " << subgraph->size_free << std::endl;
        if (!subgraph->init_crossing_matrix_if_necessary()) {
            //std::cerr << "[ERROR] Unable to generate crossing matrix." << std::endl;
            return -1;
        }

        GreedyInsertSolver upper_bound_solver(
            [](int useless_param) { return true; });
        Order upper_bound_order = upper_bound_solver.solve(*subgraph);
        graph.ub = upper_bound_order.count_crossings(*subgraph);
        apply_reduction_rules(*subgraph);
    }

    std::cout << PaceGraph::split_graphs_to_gr(splittedGraphs, isolated_nodes,
                                               graph.size_fixed)
              << std::endl;
    // TODO Reconstruct correct order solution so we can check correctness
    return 0;
}
