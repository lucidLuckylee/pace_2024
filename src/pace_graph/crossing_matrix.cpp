//
// Created by jt on 01.04.24.
//

#include "crossing_matrix.hpp"
#include "pace_graph.hpp"

bool CrossingMatrix::set_a_lt_b(int a, int b) {

    if (lt(a, b)) {
        return false;
    }

    matrix[b][a] += FIXED;
    matrix_diff[b][a] += FIXED;
    matrix_diff[a][b] -= FIXED;

    return true;
}

void CrossingMatrix::unset_a_lt_b(int a, int b) {
    matrix[b][a] -= FIXED;
    matrix_diff[b][a] -= FIXED;
    matrix_diff[a][b] += FIXED;
}

bool CrossingMatrix::lt(int a, int b) { return matrix[b][a] >= FIXED; }

bool CrossingMatrix::comparable(int a, int b) {
    return lt(a, b) || lt(b, a) || a == b;
}

bool CrossingMatrix::incomparable(int a, int b) { return !comparable(a, b); }

bool CrossingMatrix::dependent(int a, int b, int c) {
    return (incomparable(a, c) || incomparable(b, c)) && a != c && b != c;
}

void CrossingMatrix::init_crossing_matrix(PaceGraph &graph) {
    matrix.resize(graph.size_free);
    matrix_diff.resize(graph.size_free);

    for (int i = 0; i < graph.size_free; i++) {
        matrix[i] = std::vector<int>(graph.size_free, 0);
        matrix_diff[i] = std::vector<int>(graph.size_free, 0);
    }

    for (int i = 0; i < graph.size_free; i++) {
        for (int j = i + 1; j < graph.size_free; j++) {
            for (int m : graph.neighbors_free[i]) {
                for (int n : graph.neighbors_free[j]) {
                    if (m > n) {
                        matrix[i][j]++;
                    } else if (n > m) {
                        matrix[j][i]++;
                    }
                }
            }
        }
    }

    for (int i = 0; i < graph.size_free; ++i) {
        for (int j = 0; j < graph.size_free; ++j) {
            matrix_diff[i][j] = matrix[i][j] - matrix[j][i];
        }
    }
}
void CrossingMatrix::remove_free_vertices(std::vector<int> vertices) {}

bool CrossingMatrix::is_initialized() { return matrix.size() > 0; }

bool CrossingMatrix::can_initialized(PaceGraph &graph) {
    return graph.size_free <= MAX_MATRIX_SIZE;
}

void CrossingMatrix::clean() {
    matrix.clear();
    matrix_diff.clear();
}
