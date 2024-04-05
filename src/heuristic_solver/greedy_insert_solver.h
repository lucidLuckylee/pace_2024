//
// Created by Lotte on 2024-04-05.
//

#include "heuristic.hpp"
#ifndef PACE2024_GREEDY_INSERT_SOLVER_H
#define PACE2024_GREEDY_INSERT_SOLVER_H

int findBestInsertPosition(PaceGraph &graph, std::vector<int> &current_order,
                           int random_element);

class GreedyInsertSolver : Heuristic {

  public:
    explicit GreedyInsertSolver(std::function<bool(int)> has_time_left)
        : Heuristic(std::move(has_time_left)) {}

    Order solve(PaceGraph &graph) override;
};

#endif // PACE2024_GREEDY_INSERT_SOLVER_H
