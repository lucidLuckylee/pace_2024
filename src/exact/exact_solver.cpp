#include "exact_solver.h"
#include "../pace_graph/order.hpp"
#include <tuple>
#define ever (;;)
/*
 * recursive function to solve the problem with the Dujmovic-Fernau-Kaufmann algorithm for a specific k.
 */

bool ExactSolver::branch(PaceGraph &graph, Order &order, int k, int a, int b){
    order.partial_order.set_a_lt_b(a,b);
    order.partial_order.branching_step();
    if (ExactSolver::solve_k(graph, order, k - graph.crossing_matrix[a][b])) // branch into a<b
        return true; // solution found
    order.partial_order.restore();
    order.partial_order.set_a_lt_b(b, a);
    order.partial_order.branching_step();
    if (ExactSolver::solve_k(graph, order, k - graph.crossing_matrix[b][a]))
        return true;
    return false; // no solution from this node
}
bool ExactSolver::solve_k(PaceGraph &graph, Order &order, int k){
    if (k < 0) return false;
    order.apply_reduction_rules(graph, k);
    std::pair<int,int> patternB = std::make_pair(-1,-1); //pattern B: dependent 2/1 pattern {a,b}
    for (int a = 0; a < graph.size_free - 1; a++) //pattern A: incomparable i/j pattern {a,b} s.t. i+j >= 4
        for (int b = a + 1; b < graph.size_free; b++) if (order.partial_order.incomparable(a,b)){
            if (graph.crossing_matrix[a][b] + graph.crossing_matrix[b][a] >= 4){
                return branch(graph, order, k, a, b);
            }
            else{ // this relies on the fact that due to reduction rules, the only 2/1 patterns remaining
                  // are dependent and there is no ab with c_ab = 0
                if (graph.crossing_matrix[a][b] + graph.crossing_matrix[b][a] == 3) patternB = std::make_pair(a,b);
            }
        }
    if (std::get<0>(patternB) >= 0){ //second case of branching
        return branch(graph, order, k, std::get<0>(patternB), std::get<1>(patternB));
    }
    return true; //only 1/1 patterns are remaining, we can decide them arbitrarily
}
/*
 * function to find the minimum-crossing ordering for a given graph.
 */
Order ExactSolver::run(PaceGraph &graph) {
    int k = 0;
    Order order(graph.size_free);
    graph.init_crossing_matrix_if_necessary();
    order.apply_reduction_rules(graph);
    std::cout << graph.to_gr() << std::endl;
    for (int a = 0; a < graph.size_free - 1; a++) for (int b = a + 1; b < graph.size_free; b++){
            if (order.partial_order.lt(a,b)) k+= graph.crossing_matrix[a][b];
            else if (order.partial_order.lt(b,a)) k+= graph.crossing_matrix[b][a];
            else k += std::min(graph.crossing_matrix[a][b],graph.crossing_matrix[b][a]);
        } // obtain a trivial lower bound
    for ever{
    if (solve_k(graph, order, k++)) return order;
    }

}

