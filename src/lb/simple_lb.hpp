//
// Created by jt on 10.03.24.
//

#ifndef PACE2024_SIMPLE_LB_HPP
#define PACE2024_SIMPLE_LB_HPP

#include "../pace_graph/pace_graph.hpp"

enum class SearchStrategyForConflicts { MATRIX, NEIGHBORS, BMI };

class SimpleLBParameter {
  public:
    bool usePotentialMatrix = true;
    bool testForceChoiceOfConflicts = false;

    int nrOfConflictsToUsePseudoRandom = 1000000;
    int numberOfIterationsForConflictOrder = 100;
    SearchStrategyForConflicts searchStrategyForConflicts =
        SearchStrategyForConflicts::BMI;
};

long simpleLB(PaceGraph &graph, SimpleLBParameter &parameter);

#endif // PACE2024_SIMPLE_LB_HPP
