#ifndef PACE2024_BOOL_CROSSING_MATRIX_HPP
#define PACE2024_BOOL_CROSSING_MATRIX_HPP

#include <vector>

#define MAX_MATRIX_SIZE 200000

class PaceGraph;

class CrossingMatrix {

  public:
    std::vector<std::vector<bool>> matrix;
    std::vector<std::vector<int>> matrix_diff;

    ~CrossingMatrix();

    bool set_a_lt_b(int a, int b);
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

#endif // BOOL_PACE2024_CROSSING_MATRIX_HPP
