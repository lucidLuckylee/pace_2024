#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    Order order(graph.size_free);
    graph.init_crossing_matrix_if_necessary();
    // TODO Use heuristic to find upper_bound
    // order.apply_reduction_rules(graph, upper_bound);
    // order.apply_reduction_rules(graph);
    std::cout << graph.to_gr() << std::endl;

    // TODO Reconstruct correct order solution so we can check correctness
    return 0;
}
