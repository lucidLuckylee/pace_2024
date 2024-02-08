#ifndef PACE2024_LOCAL_SEARCH_H
#define PACE2024_LOCAL_SEARCH_H

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"

/**
 * Tries to improve a given order by performing local search steps.
 * @param graph input graph
 * @param order current order. It will be modified in place
 * @return the improvement in the cost of the order
 */
int local_search(PaceGraph &graph, Order &order);

#endif // PACE2024_LOCAL_SEARCH_H