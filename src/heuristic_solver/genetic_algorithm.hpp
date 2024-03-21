#ifndef PACE2024_GENETIC_ALGORITHM_HPP
#define PACE2024_GENETIC_ALGORITHM_HPP

#include <utility>

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "heuristic_solver.hpp"

class GeneticHeuristic : public Heuristic {
  public:
    explicit GeneticHeuristic(std::function<bool(int)> has_time_left)
        : Heuristic(std::move(has_time_left)) {}
    Order solve(PaceGraph &graph) override;
};

#endif // PACE2024_GENETIC_ALGORITHM_HPP
