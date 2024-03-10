#include "../pace_graph/order.hpp"
#include "simple_lb.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);

    auto splittedGraphs = std::get<0>(graph.splitGraphOn0Splits());

    int lb = 0;
    for (auto &g : splittedGraphs) {
        lb += simpleLB(g);
    }
    std::cout << lb << std::endl;

    return 0;
}