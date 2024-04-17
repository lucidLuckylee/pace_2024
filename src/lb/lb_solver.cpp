
#include "lb_solver.hpp"
#include "simple_lb.hpp"
#include <iostream>
long LBSolver::run(PaceGraph &graph) {
    SimpleLBParameter parameter;
    return simpleLB(graph, parameter);
}
void LBSolver::finish(PaceGraph &graph,
                      std::vector<std::unique_ptr<PaceGraph>> &subgraphs,
                      std::vector<long> &results,
                      std::vector<int> &isolated_nodes) {
    long lb = graph.cost_through_deleted_nodes;

    for (auto &g : subgraphs) {
        lb += g->cost_through_deleted_nodes;
    }

    for (auto &r : results) {
        lb += r;
    }
    std::cout << lb << std::endl;
}
