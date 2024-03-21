#ifndef PACE2024_GENETIC_ALGORITHM_HPP
#define PACE2024_GENETIC_ALGORITHM_HPP

#include <utility>

#include "../pace_graph/order.hpp"
#include "heuristic.hpp"

class GeneticHeuristic : public Heuristic {
  public:
    explicit GeneticHeuristic(std::function<bool()> has_time_left)
        : Heuristic(std::move(has_time_left)) {}
    Order solve(PaceGraph &graph) override;
};

#endif // PACE2024_GENETIC_ALGORITHM_HPP
