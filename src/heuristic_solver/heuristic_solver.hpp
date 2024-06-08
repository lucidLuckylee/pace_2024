#ifndef PACE2024_HEURISTIC_SOLVER_HPP
#define PACE2024_HEURISTIC_SOLVER_HPP

#include <utility>

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"

Order largeGraphHeuristic(PaceGraph &graph,
                          const std::function<bool()> &has_time_left,
                          const std::function<double()> &time_percentage_past);

class HeuristicSolver : public SolutionSolver {

  protected:
    Order run(PaceGraph &graph) override;

  public:
    explicit HeuristicSolver(std::chrono::milliseconds limit =
                                 std::chrono::milliseconds(1000 * 60 * 5 -
                                                           1000 * 15))
        : SolutionSolver(limit) {}
};

#endif // PACE2024_HEURISTIC_SOLVER_HPP
