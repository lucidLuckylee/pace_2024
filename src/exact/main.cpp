#include "feedback_edge_set_solver.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    FeedbackEdgeSetSolver solver;
    solver.solve(graph);
    return 0;
}
