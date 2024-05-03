//
// Created by jt on 09.04.24.
//

#ifndef PACE2024_FEEDBACK_EDGE_SET_HEURISTIC_HPP
#define PACE2024_FEEDBACK_EDGE_SET_HEURISTIC_HPP

#include "feedback_edge_set_solver.hpp"

class FeedbackEdgeHeuristicParameter {
  public:
    double priority = 0.05;
    double restriction = 0.35;
    double improvement = 0.15;
    double search_magnitude = 0.3;
    int max_iterations = 100;
    int imp_iterations = 400;
};

void approximateFeedbackEdgeSet(FeedbackEdgeInstance &instance,
                                FeedbackEdgeHeuristicParameter &parameter);

#endif // PACE2024_FEEDBACK_EDGE_SET_HEURISTIC_HPP
