#include "feedback_edge_set_solver.hpp"
#include <iostream>

int main() {

    FESParameter fes_parameter;
    PaceGraph graph = PaceGraph::from_gr(std::cin);
    FeedbackEdgeSetSolver solver(fes_parameter);
    solver.solve(graph);
    return 0;
}
