//
// Created by jt on 10.03.24.
//

#ifndef PACE2024_SIMPLE_LB_HPP
#define PACE2024_SIMPLE_LB_HPP

#include "../pace_graph/pace_graph.hpp"

class SimpleLBParameter {
  public:
    bool usePotentialMatrix = true;
    int numberOfIterationsForConflictOrder = 100;
};

int simpleLB(PaceGraph &graph, SimpleLBParameter &parameter);

#endif // PACE2024_SIMPLE_LB_HPP
