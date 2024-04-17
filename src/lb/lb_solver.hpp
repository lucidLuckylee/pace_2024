//
// Created by jt on 19.03.24.
//

#ifndef PACE2024_LB_SOLVER_HPP
#define PACE2024_LB_SOLVER_HPP

#include "../pace_graph/solver.hpp"

class LBSolver : public Solver<long> {
  protected:
    void finish(PaceGraph &graph,
                std::vector<std::unique_ptr<PaceGraph>> &subgraphs,
                std::vector<long> &results,
                std::vector<int> &isolated_nodes) override;
    long run(PaceGraph &graph) override;

  public:
    LBSolver(std::chrono::milliseconds limit = std::chrono::milliseconds::max())
        : Solver<long>(limit) {}
};

#endif // PACE2024_LB_SOLVER_HPP
