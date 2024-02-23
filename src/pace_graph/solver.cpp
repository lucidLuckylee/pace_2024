#include "solver.hpp"
#include "pace_graph.hpp"
#include <iostream>

Solver::Solver(PaceGraph &graph) {
    Solver::instance = this;
    this->graph = &graph;
    std::signal(SIGTERM, Solver::signal_handler);
};

Solver *Solver::instance = nullptr;

void Solver::signal_handler(int signal_num) {
    if (Solver::instance) {
        Order result = Solver::instance->terminate();
        std::cout << result.convert_to_real_node_id(*Solver::instance->graph)
                  << std::endl;
        exit(0);
    }
    exit(signal_num);
}
