#ifndef PACE2024_CHEAP_HEURISTICS_H
#define PACE2024_CHEAP_HEURISTICS_H
#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"

enum MeanTypeAlgo { average, median, sum_along_crossing };

class MeanPositionSolver : Solver {
  public:
    MeanTypeAlgo meanType;
    std::vector<std::tuple<int, double>> average_position;

    explicit MeanPositionSolver(PaceGraph &graph, MeanTypeAlgo meanType);

    Order terminate();

    Order solve();
};

#endif // PACE2024_CHEAP_HEURISTICS_H
