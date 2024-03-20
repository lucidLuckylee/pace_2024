#ifndef PACE2024_CHEAP_HEURISTICS_H
#define PACE2024_CHEAP_HEURISTICS_H
#include <utility>

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"
#include "heuristic_solver.hpp"

enum MeanTypeAlgo { average, median, sum_along_crossing };

class MeanPositionParameter {
  public:
    MeanTypeAlgo meanType = median;
    bool useJittering = true;
    int jitterIterations = 1000;
};

class MeanPositionSolver : Heuristic {
  private:
    Order jittering(PaceGraph &graph);

  public:
    MeanPositionParameter meanPositionParameter;

    explicit MeanPositionSolver(std::function<bool()> has_time_left,
                                MeanPositionParameter meanPositionParameter)
        : Heuristic(std::move(has_time_left)),
          meanPositionParameter(meanPositionParameter) {}

    Order solve(PaceGraph &graph) override;
};

#endif // PACE2024_CHEAP_HEURISTICS_H
