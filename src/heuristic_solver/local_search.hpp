#ifndef PACE2024_LOCAL_SEARCH_HPP
#define PACE2024_LOCAL_SEARCH_HPP

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"

enum class SiftingType {
    None,
    Random,
    DegreeOrder,
    DegreeOrderReverse,
};

enum class SiftingInsertionType {
    First,
    Random,
};

class LocalSearchParameter {
  public:
    SiftingType siftingType = SiftingType::None;
    SiftingInsertionType siftingInsertionType = SiftingInsertionType::First;
};

/**
 * Tries to improve a given order by performing local search steps.
 * @param graph input graph
 * @param order current order. It will be modified in place
 * @return the improvement in the cost of the order
 */
int local_search(PaceGraph &graph, Order &order,
                 LocalSearchParameter &parameter);

#endif // PACE2024_LOCAL_SEARCH_HPP