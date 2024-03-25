
#include "lb_solver.hpp"
#include "simple_lb.hpp"
long LBSolver::run(PaceGraph &graph) {
    SimpleLBParameter parameter;
    return simpleLB(graph, parameter);
}
void LBSolver::finish(PaceGraph &graph, std::vector<PaceGraph> &subgraphs,
                      std::vector<long> &results,
                      std::vector<int> &isolated_nodes) {
    long lb = 0;
    for (auto &r : results) {
        lb += r;
    }
    std::cout << lb << std::endl;
}