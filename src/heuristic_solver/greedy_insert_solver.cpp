#include <random>
#include <unordered_set>

#include "../pace_graph/order.hpp"
#include "greedy_insert_solver.hpp"

int findBestInsertPosition(PaceGraph &graph, std::vector<int> &current_order,
                           int random_element) {
    // find best position in existing order
    int best_position = 0;
    int least_crossings = 0;
    int number_of_crossing_left = 0;
    int number_of_crossing_right = 0;

    for (int i = 0; i <= current_order.size(); ++i) {
        if (i == 0) {
            for (int v : current_order) {
                number_of_crossing_right +=
                    graph.crossing.matrix[random_element][v];
            }
            least_crossings = number_of_crossing_right;
        } else {
            number_of_crossing_left +=
                graph.crossing.matrix[current_order[i - 1]][random_element];
            number_of_crossing_right -=
                graph.crossing.matrix[random_element][current_order[i - 1]];

            if (number_of_crossing_right + number_of_crossing_left <
                least_crossings) {
                best_position = i;
                least_crossings =
                    number_of_crossing_right + number_of_crossing_left;
            }
        }
    }
    return best_position;
}

Order GreedyInsertSolver::solve(PaceGraph &graph) {
    std::vector<int> current_order;

    graph.init_crossing_matrix_if_necessary();

    // initialize set with vertices not yet placed in the order
    std::unordered_set<int> vertices_not_inserted;
    for (int i = 0; i < graph.size_free; ++i) {
        vertices_not_inserted.insert(i);
    }

    // Seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Generate a random index

    while (!vertices_not_inserted.empty()) {

        std::uniform_int_distribution<> dis(0,
                                            vertices_not_inserted.size() - 1);

        // get randomly selected element
        auto it = std::next(vertices_not_inserted.begin(), dis(gen));
        int random_element = *it;

        int best_position =
            findBestInsertPosition(graph, current_order, random_element);

        // insert vertex into order and remove from unordered set
        current_order.insert(current_order.begin() + best_position,
                             random_element);
        vertices_not_inserted.erase(random_element);
    }

    return Order(current_order);
}
