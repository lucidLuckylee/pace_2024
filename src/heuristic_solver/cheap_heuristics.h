//
// Created by Lotte on 2024-02-13.
//

#ifndef PACE2024_CHEAP_HEURISTICS_H
#define PACE2024_CHEAP_HEURISTICS_H
#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"

enum MeanTypeAlgo { average, median };

Order mean_position_algorithm(PaceGraph &graph, MeanTypeAlgo meanType);

#endif // PACE2024_CHEAP_HEURISTICS_H
