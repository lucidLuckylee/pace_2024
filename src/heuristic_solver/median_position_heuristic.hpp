#ifndef PACE2024_CHEAP_HEURISTICS_H
#define PACE2024_CHEAP_HEURISTICS_H
#include <utility>

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "heuristic.hpp"

enum MeanTypeAlgo { average, median, sum_along_crossing };

class CheapHeuristicsParameter {
  public:
    MeanTypeAlgo meanType = median;
};

class MeanPositionSolver : Heuristic {
  public:
    CheapHeuristicsParameter cheapHeuristicsParameter;

    explicit MeanPositionSolver(
        std::function<bool()> has_time_left,
        CheapHeuristicsParameter cheapHeuristicsParameter)
        : Heuristic(std::move(has_time_left)),
          cheapHeuristicsParameter(cheapHeuristicsParameter) {}

    Order solve(PaceGraph &graph) override;
};

#endif // PACE2024_CHEAP_HEURISTICS_H
