
#ifndef PACE2024_EXACT_SOLVER_H
#define PACE2024_EXACT_SOLVER_H
#include <utility>

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"

class ExactSolver : public SolutionSolver {

  public:

explicit ExactSolver(std::chrono::milliseconds limit = std::chrono::milliseconds(1000 * 60 - 1000))
    : SolutionSolver(limit) {}
    static bool solve_k(PaceGraph &graph, Order &order, int k);
    static bool branch(PaceGraph &graph, Order &order, int k, int a, int b);
    Order run(PaceGraph &graph) override;
};

#endif // PACE2024_EXACT_SOLVER_H
