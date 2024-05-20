#ifndef PACE2024_DATA_REDUCTION_RULES_H
#define PACE2024_DATA_REDUCTION_RULES_H

#include "../pace_graph/pace_graph.hpp"

bool rr1(PaceGraph &graph);
void rr2(PaceGraph &graph);
bool rr3(PaceGraph &graph);
bool rrlarge(PaceGraph &graph);
bool rrlo1(PaceGraph &graph);
bool rrlo2(PaceGraph &graph);
bool rrtransitive(PaceGraph &graph);
void apply_reduction_rules(PaceGraph &graph);

#endif // PACE2024_DATA_REDUCTION_RULES_H
