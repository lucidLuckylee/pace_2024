//
// Created by jt on 01.03.24.
//

#ifndef PACE2024_ILP_SOLVER_HPP
#define PACE2024_ILP_SOLVER_HPP

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"

Order ilpSolver(PaceGraph &graph);

class ILPSolver : public SolutionSolver {
  protected:
    Order run(PaceGraph &graph) override { return ilpSolver(graph); }

  public:
    explicit ILPSolver(
        std::chrono::milliseconds limit = std::chrono::milliseconds::max())
        : SolutionSolver(limit, true) {}
};

#endif // PACE2024_ILP_SOLVER_HPP
