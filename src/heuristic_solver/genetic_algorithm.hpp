#ifndef PACE2024_GENETIC_ALGORITHM_HPP
#define PACE2024_GENETIC_ALGORITHM_HPP

#include <utility>

#include "../pace_graph/order.hpp"
#include "heuristic.hpp"
#include "local_search.hpp"

class GeneticHeuristicParameter {

  private:
    GeneticHeuristicParameter() {}

  public:
    GeneticHeuristicParameter(const GeneticHeuristicParameter &) = delete;
    GeneticHeuristicParameter &
    operator=(const GeneticHeuristicParameter &) = delete;

    static GeneticHeuristicParameter &getInstance() {
        static GeneticHeuristicParameter instance;
        return instance;
    }

    void parseArgs(int argc, char *argv[]) {
        forceMoveAllDirectNodesAfterIterationWithNoImprovement =
            std::strtol(argv[1], nullptr, 0);
        numberOfForceSwapPositions = std::strtol(argv[2], nullptr, 0);
        numberOfForceSwapStepSize = std::strtol(argv[3], nullptr, 0);
    }

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
    explicit GeneticHeuristic(std::function<bool(int)> has_time_left)
        : Heuristic(std::move(has_time_left)) {}
    Order solve(PaceGraph &graph) override;
};

#endif // PACE2024_GENETIC_ALGORITHM_HPP
