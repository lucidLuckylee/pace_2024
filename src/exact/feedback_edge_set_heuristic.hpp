//
// Created by jt on 09.04.24.
//

#ifndef PACE2024_FEEDBACK_EDGE_SET_HEURISTIC_HPP
#define PACE2024_FEEDBACK_EDGE_SET_HEURISTIC_HPP

#include "feedback_edge_set_solver.hpp"

void approximateFeedbackEdgeSet(FeedbackEdgeInstance &instance);

void greedyApproximateFeedbackEdgeSet(FeedbackEdgeInstance &instance);
void globalApproximateFeedbackEdgeSet(FeedbackEdgeInstance &instance);
void localSearchFeedbackEdgeSet(FeedbackEdgeInstance &instance);

#endif // PACE2024_FEEDBACK_EDGE_SET_HEURISTIC_HPP
