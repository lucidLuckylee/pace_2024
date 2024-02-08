#ifndef PACE_GRAPH_HPP
#define PACE_GRAPH_HPP

#include <fstream>
#include <list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

class PaceGraph {
  public:
    /** Number of size_free vertices
    We name this vertices: [0,..., size_free - 1]
     */
    int size_free;

    /** We name this vertices: [0,..., size_fixed - 1]
     */
    int size_fixed;

    /**  Saves the neighbors of a vertex i \in [0,..., size_free - 1] in
     * neighbors_free[i]. These neighbors are sorted in ascending order.
     */
    std::vector<std::vector<int>> neighbors_free;

    /** Saves the neighbors of a vertex i \in [0,..., size_fixed - 1] in
     * neighbors_fixed[i]. These neighbors are sorted in ascending order.
     */
    std::vector<std::vector<int>> neighbors_fixed;

    /** Saving the numbers of crossing between two vertices i and j \in [0,...,
     * size_free - 1] in crossing_matrix[i][j] when i comes before j.
     */
    std::vector<std::vector<int>> crossing_matrix;

    /** Saving the numbers of crossing between two vertices i and j \in [0,...,
     * size_free - 1] in crossing_matrix[j][i] when i comes before j.
     *
     * It's the transposed of crossing_matrix, so crossing_matrix[i][j] ==
     * crossing_matrix_transposed[j][i]
     */
    std::vector<std::vector<int>> crossing_matrix_transposed;

    /**
     *
     * @param a the number of size_fixed vertices
     * @param b the number of size_free vertices
     * @param edges an array of tuples (u, v) representing the edges of the
     * graph. u must be always in [0,..., a - 1] and v must be always in [0,...,
     * b - 1]
     */
    PaceGraph(int a, int b, std::vector<std::tuple<int, int>> edges);

    /**
     * To read more about the .gr file format, see:
     * https://pacechallenge.org/2024/io/
     * @param gr a file stream (e.g. stdin or a file stream from a file)
     */
    static PaceGraph from_gr(std::ifstream &gr);

    static PaceGraph from_file(std::string filePath);

    std::string to_gr();

    std::string print_crossing_matrix();
};

#endif // PACE_GRAPH_HPP
