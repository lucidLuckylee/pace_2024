#include "../pace_graph/pace_graph.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    graph.init_crossing_matrix_if_necessary();
    //apply_reduction_rules(graph);
    graph.crossing.print();

    return 0;
}
