//
// Created by jt on 01.04.24.
//

#ifndef PACE2024_CROSSING_MATRIX_HPP
#define PACE2024_CROSSING_MATRIX_HPP

#include <vector>

#define FIXED 1000000
#define MAX_MATRIX_SIZE 10000

class PaceGraph;

class CrossingMatrix {
  private:
    bool is_init = false;

  public:
    std::vector<int *> matrix;
    std::vector<int *> matrix_diff;

    ~CrossingMatrix();

    bool set_a_lt_b(int a, int b);
    void unset_a_lt_b(int a, int b);
    bool comparable(int a, int b);
    bool incomparable(int a, int b);

    bool dependent(int a, int b, int c);
    bool dependentInPartialOrder(int a, int b);

    bool lt(int a, int b);

    void init_crossing_matrix(PaceGraph &graph);
    bool can_initialized(PaceGraph &graph);
    bool is_initialized();

    void remove_free_vertices(std::vector<int> &vertices_to_remove);
    void clean();
    void print();
    void print_averaged();
};

#endif // PACE2024_CROSSING_MATRIX_HPP
