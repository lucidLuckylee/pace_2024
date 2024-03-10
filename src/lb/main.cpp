#include "../pace_graph/order.hpp"
#include "simple_lb.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    graph.init_crossing_matrix_if_necessary();

    int lb = simpleLB(graph);

    std::cout << lb << std::endl;

    return 0;
}