#ifndef PACE2024_HEURISTIC_SOLVER_HPP
#define PACE2024_HEURISTIC_SOLVER_HPP

#include <utility>

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"

class HeuristicSolver : public SolutionSolver {
  protected:
    Order run(PaceGraph &graph) override;

  public:
    explicit HeuristicSolver(std::chrono::milliseconds limit =
                                 std::chrono::milliseconds(1000 * 60 - 1000))
        : SolutionSolver(limit) {}
};

#endif // PACE2024_HEURISTIC_SOLVER_HPP
