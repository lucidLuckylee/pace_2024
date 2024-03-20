#ifndef PACE2024_CHEAP_HEURISTICS_H
#define PACE2024_CHEAP_HEURISTICS_H
#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"

enum MeanTypeAlgo { average, median, sum_along_crossing };

class CheapHeuristicsParameter {
  public:
    MeanTypeAlgo meanType = median;
    int jitterIterations = 1000;
};

class MeanPositionSolver {
  public:
    CheapHeuristicsParameter cheapHeuristicsParameter;

    explicit MeanPositionSolver(
        CheapHeuristicsParameter cheapHeuristicsParameter);

    Order solve(PaceGraph &graph);
};

#endif // PACE2024_CHEAP_HEURISTICS_H
