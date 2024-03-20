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
                                 std::chrono::milliseconds(1000 * 15 - 10))
        : SolutionSolver(limit) {}
};

class Heuristic {
  protected:
    std::function<bool()> has_time_left;

  public:
    explicit Heuristic(std::function<bool()> has_time_left)
        : has_time_left(std::move(has_time_left)) {}
    virtual Order solve(PaceGraph &graph) = 0;
};

#endif // PACE2024_HEURISTIC_SOLVER_HPP
