#include "bool_crossing_matrix.hpp"
#include "pace_graph.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

bool CrossingMatrix::set_a_lt_b(int a, int b) {

    if (lt(a, b) || a == b) {
        return false;
    }

    matrix[a][b] = 0;
    return true;
}

bool CrossingMatrix::lt(int a, int b) { return !matrix[a][b]; }

bool CrossingMatrix::comparable(int a, int b) {
    return a == b || lt(a, b) || lt(b, a);
}

bool CrossingMatrix::incomparable(int a, int b) { return !comparable(a, b); }

bool CrossingMatrix::dependent(int a, int b, int c) {
    return (incomparable(a, c) || incomparable(b, c)) && a != c && b != c;
}

void CrossingMatrix::init_crossing_matrix(PaceGraph &graph) {
    matrix = std::vector(graph.size_free, std::vector(graph.size_free, false));
    for (int i = 0; i < graph.size_free; i++) {
        for (int j = i + 1; j < graph.size_free; j++) {
            if (graph.neighbors_free[j].size() > 0 &&
                graph.neighbors_free[i].size() > 0) {
                matrix[i][j] = !(graph.neighbors_free[j][0] >=
                                 graph.neighbors_free[i][graph.neighbors_free[i].size() - 1]);
                matrix[j][i] = !(graph.neighbors_free[i][0] >=
                                 graph.neighbors_free[j][graph.neighbors_free[j].size() - 1]);
            }
        }
    }
}

void CrossingMatrix::remove_free_vertices(
    std::vector<int> &vertices_to_remove) {

    std::sort(vertices_to_remove.begin(), vertices_to_remove.end());

    for (int i = vertices_to_remove.size() - 1; i >= 0; --i) {
        matrix.erase(matrix.begin() + vertices_to_remove[i]);
    }

    for (int i = vertices_to_remove.size() - 1; i >= 0; --i) {
        for (int j = 0; j < matrix.size(); ++j) {
            matrix[j].erase(matrix[j].begin() + vertices_to_remove[i]);
        }
    }
}

bool CrossingMatrix::is_initialized() { return matrix.size() > 0; }

bool CrossingMatrix::can_initialized(PaceGraph &graph) {
    return graph.size_free <= MAX_MATRIX_SIZE;
}

void CrossingMatrix::print() {
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix.size(); j++) {
            std::cout << matrix[i][j] << ",";
        }
        std::cout << std::endl;
    }
}

void CrossingMatrix::clean() {}
CrossingMatrix::~CrossingMatrix() {}
