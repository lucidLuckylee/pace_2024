#ifndef PACE2024_CHEAP_HEURISTICS_H
#define PACE2024_CHEAP_HEURISTICS_H
#include <utility>

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "heuristic.hpp"

enum MeanTypeAlgo { average, median, sum_along_crossing };

class MeanPositionParameter {
  public:
    MeanTypeAlgo meanType = median;
    bool useJittering = true;
    bool useLocalSearch = true;
    int jitterIterations = 3;
};

class MeanPositionSolver : Heuristic {
  private:
    Order jittering(PaceGraph &graph, int iteration);
    void improveOrderWithSwapping(PaceGraph &graph, Order &order,
                                  int iteration);

  public:
    MeanPositionParameter meanPositionParameter;

    explicit MeanPositionSolver(std::function<bool(int)> has_time_left,
                                MeanPositionParameter meanPositionParameter)
        : Heuristic(std::move(has_time_left)),
          meanPositionParameter(meanPositionParameter) {}

    Order solve(PaceGraph &graph, char** argv) override;
};

#endif // PACE2024_CHEAP_HEURISTICS_H
