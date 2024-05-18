#ifndef PACE2024_GENETIC_ALGORITHM_HPP
#define PACE2024_GENETIC_ALGORITHM_HPP

#include <utility>

#include "../pace_graph/order.hpp"
#include "heuristic.hpp"
#include "local_search.hpp"

class GeneticHeuristicParameter {
  public:
    int forceMoveAllDirectNodesAfterIterationWithNoImprovement = 100;
    int numberOfForceSwapPositions = 20;
    int numberOfForceSwapStepSize = 1;

    SiftingType siftingTypeInitialSearch = SiftingType::Random;
    SiftingInsertionType siftingInsertionTypeInitialSearch =
        SiftingInsertionType::Last;

    SiftingType siftingTypeImprovementSearch = SiftingType::Random;
    SiftingInsertionType siftingInsertionTypeImprovementSearch =
        SiftingInsertionType::Last;
};

class GeneticHeuristic : public Heuristic {
  public:
    GeneticHeuristicParameter geneticHeuristicParameter;

    explicit GeneticHeuristic(
        std::function<bool(int)> has_time_left,
        GeneticHeuristicParameter geneticHeuristicParameter)
        : geneticHeuristicParameter(geneticHeuristicParameter),
          Heuristic(std::move(has_time_left)) {}
    Order solve(PaceGraph &graph, char **argv) override;
};

#endif // PACE2024_GENETIC_ALGORITHM_HPP
