#ifndef PACE2024_GENETIC_ALGORITHM_HPP
#define PACE2024_GENETIC_ALGORITHM_HPP

#include <utility>

#include "../pace_graph/order.hpp"
#include "heuristic.hpp"

class GeneticHeuristicParameter {
  public:
    int forceMoveAllDirectNodesAfterIterationWithNoImprovement = 1000;
};

class GeneticHeuristic : public Heuristic {
  public:
    GeneticHeuristicParameter geneticHeuristicParameter;

    explicit GeneticHeuristic(
        std::function<bool(int)> has_time_left,
        GeneticHeuristicParameter geneticHeuristicParameter)
        : geneticHeuristicParameter(geneticHeuristicParameter),
          Heuristic(std::move(has_time_left)) {}
    Order solve(PaceGraph &graph) override;
};

#endif // PACE2024_GENETIC_ALGORITHM_HPP
