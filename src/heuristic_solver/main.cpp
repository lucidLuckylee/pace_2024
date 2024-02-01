// heuristic_solver/main.cpp
#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include <deque>
#include <iostream>

int main() {
    try {
        // Example: Creating a PaceGraph from a file
        PaceGraph graph_from_file =
            PaceGraph::from_file("../data/cycle_8_shuffled.gr");

        // Create an Order with a std::list Container
        Order<std::list<int>> test_order(1, 4);
        Order<std::deque<int>> test_order_2(test_order);
        Order<std::vector<int>> test_order_3(std::vector({1, 2, 3, 4}));
        std::cout << "test_order: " << test_order.to_string() << std::endl;
        std::cout << "test_order_2: " << test_order_2.to_string() << std::endl;
        std::cout << "test_order_3: " << test_order_3.to_string() << std::endl;

        std::cout << "PaceGraph objects created successfully." << std::endl;
        std::cout << "Graph in gr format: " << std::endl;
        std::cout << graph_from_file.to_gr() << std::endl;

        std::cout << "Crossing matrix: " << std::endl;

        graph_from_file.print_crossing_matrix();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
