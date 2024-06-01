#include "feedback_edge_set_solver.hpp"
#include <iostream>

int main() {
    PaceGraph graph = PaceGraph::from_gr(std::cin);

    FESParameter fes_parameter;
    fes_parameter.useFastHeuristic = graph.is_cutwidth_graph;

    FeedbackEdgeSetSolver solver(fes_parameter);
    solver.solve(graph);
    return 0;
}
