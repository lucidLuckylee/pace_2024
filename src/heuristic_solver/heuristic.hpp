//
// Created by jt on 21.03.24.
//

#ifndef PACE2024_HEURISTIC_HPP
#define PACE2024_HEURISTIC_HPP

#include "../pace_graph/order.hpp"
#include <functional>

class Heuristic {
  protected:
    std::function<bool(int)> has_time_left;

  public:
    explicit Heuristic(std::function<bool(int)> has_time_left)
        : has_time_left(std::move(has_time_left)) {}
    virtual Order solve(PaceGraph &graph, char **argv) = 0;
};

#endif // PACE2024_HEURISTIC_HPP
