// heuristic_solver/main.cpp
#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include <deque>
#include <iostream>

int main() {
    try {
        // Example: Creating a PaceGraph from a file
        PaceGraph graph_from_file =
            PaceGraph::from_file("../data/grid_9_shuffled.gr");

        // Create an Order with a std::list Container
        std::cout << "PaceGraph objects created successfully." << std::endl;
        std::cout << "Graph in gr format: " << std::endl;
        std::cout << graph_from_file.to_gr() << std::endl;

        std::cout << "Crossing matrix: " << std::endl;

        graph_from_file.print_crossing_matrix();

        Order test_order(graph_from_file.size_free);
        test_order.swap_by_vertices(0, 1);
        test_order.swap_by_vertices(4, 1);
        test_order.swap_by_vertices(3, 4);

        // test_order.swap_by_vertices(1, 2);
        // test_order.swap_by_vertices(3, 1);
        // test_order.swap_by_vertices(3, 2);

        std::cout << test_order.to_string() << std::endl;
        std::cout << "Number of crossings: "
                  << test_order.count_crossings(graph_from_file) << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
