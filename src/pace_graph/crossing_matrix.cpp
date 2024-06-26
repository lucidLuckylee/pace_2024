//
// Created by jt on 01.04.24.
//

#include "crossing_matrix.hpp"
#include "order.hpp"
#include "pace_graph.hpp"
#include <algorithm>
#include <iostream>

bool CrossingMatrix::set_a_lt_b(int a, int b) {

    if (lt(a, b) || a == b) {
        return false;
    }

    matrix[b][a] += FIXED;
    matrix_diff[b][a] += FIXED;
    matrix_diff[a][b] -= FIXED;

    return true;
}

void CrossingMatrix::unset_a_lt_b(int a, int b) {
    if (a == b) {
        return;
    }
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
        matrix[i] = new int[graph.size_free];
        matrix_diff[i] = new int[graph.size_free];
    }

    for (int i = 0; i < graph.size_free; i++) {
        matrix[i][i] = 0;
        matrix_diff[i][i] = 0;
        for (int j = i + 1; j < graph.size_free; j++) {
            auto [a, b] = graph.calculatingCrossingNumber(i, j);

            matrix[i][j] = a;
            matrix[j][i] = b;

            matrix_diff[i][j] = a - b;
            matrix_diff[j][i] = -matrix_diff[i][j];
        }
    }

    is_init = true;
}
void CrossingMatrix::remove_free_vertices(
    std::vector<int> &vertices_to_remove) {

    std::sort(vertices_to_remove.begin(), vertices_to_remove.end());

    for (int j = 0; j < matrix.size(); ++j) {
        int backward_copy = 1;
        int current_vertices_to_remove_index = 1;
        for (int i = vertices_to_remove[0] + 1; i < matrix.size(); ++i) {
            if (current_vertices_to_remove_index < vertices_to_remove.size() &&
                vertices_to_remove[current_vertices_to_remove_index] == i) {
                current_vertices_to_remove_index++;
                backward_copy++;
            } else {
                matrix[j][i - backward_copy] = matrix[j][i];
                matrix_diff[j][i - backward_copy] = matrix_diff[j][i];
            }
        }
    }

    for (const auto &v : vertices_to_remove) {
        delete[] matrix[v];
        delete[] matrix_diff[v];
    }

    for (int i = vertices_to_remove.size() - 1; i >= 0; i--) {
        matrix.erase(matrix.begin() + vertices_to_remove[i]);
        matrix_diff.erase(matrix_diff.begin() + vertices_to_remove[i]);
    }
}

bool CrossingMatrix::is_initialized() { return is_init; }

bool CrossingMatrix::can_initialized(PaceGraph &graph) {
    return graph.size_free <= MAX_MATRIX_SIZE;
}

void CrossingMatrix::clean() {
    is_init = false;
    for (int i = 0; i < matrix.size(); i++) {
        delete[] matrix[i];
        delete[] matrix_diff[i];
    }

    matrix.clear();
    matrix_diff.clear();
}
CrossingMatrix::~CrossingMatrix() { clean(); }

void CrossingMatrix::print() {
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix.size(); j++) {
            std::cout << matrix[i][j] << ", ";
        }
        std::cout << std::endl;
    }
}

void CrossingMatrix::print_in_order(Order &order) {
    int rows = matrix.size();
    int cols = matrix.size();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << matrix[order.position_to_vertex[i]]
                               [order.position_to_vertex[j]]
                      << ",";
        }
        std::cout << std::endl;
    }
}
