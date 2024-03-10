// heuristic_solver/main.cpp
#include "../pace_graph/order.hpp"
#include "../pace_graph/segment_tree.hpp"
#include "cheap_heuristics.hpp"
#include "genetic_algorithm.hpp"
#include <iostream>
#include <ostream>

int main() {
    std::cout << "# Reading in graph" << std::endl;
    PaceGraph graph_from_file =
        // PaceGraph::from_file("data/heuristic_public/99.gr");
        PaceGraph::from_gr(std::cin);

    /*MeanPositionSolver *solver =
        new MeanPositionSolver(graph_from_file, MeanTypeAlgo::median);
    Order sol = solver->solve();*/

    auto val = graph_from_file.splitGraphOn0Splits();
    auto splittedGraphs = std::get<0>(val);
    auto isolated_nodes = std::get<1>(val);

    for (int u : isolated_nodes) {
        std::cout << graph_from_file.free_real_names[u] << std::endl;
    }

    int overallTimeMs = 10 * 1000;
    long crossings = 0;

    auto start_time = std::chrono::steady_clock::now();

    for (int i = 0; i < splittedGraphs.size(); i++) {
        auto g = splittedGraphs[i];

        int msLeft = overallTimeMs -
                     std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now() - start_time)
                         .count();

        int sizeForAllUpcomingSegments = 0;
        for (int j = i; j < splittedGraphs.size(); j++) {
            sizeForAllUpcomingSegments += splittedGraphs[j].size_free;
        }

        double percentageForThisSegment =
            double(g.size_free) / sizeForAllUpcomingSegments;

        Order sol = genetic_algorithm(g, msLeft * percentageForThisSegment);
        crossings += sol.count_crossings(g);
        std::cout << sol.convert_to_real_node_id(g) << std::endl;
    }

    std::cout << "#Crossings: " << crossings << std::endl;

    return 0;
}
