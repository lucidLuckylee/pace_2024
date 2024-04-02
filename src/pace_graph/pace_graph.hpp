#ifndef PACE_GRAPH_HPP
#define PACE_GRAPH_HPP

#include "crossing_matrix.hpp"
#include <fstream>
#include <list>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

class PaceGraph {
  private:
    static std::unordered_map<int, int> createMap(int limit, int offset) {
        std::unordered_map<int, int> result;
        for (int i = 0; i < limit; ++i) {
            result[i] = i + offset + 1;
        }
        return result;
    }

  public:
    long lb = 0;
    long ub = 1000000000;

    long cost_through_deleted_nodes = 0;

    /** Number of size_free vertices
            We name these vertices: [0,..., size_free - 1]
             */
    int size_free;

    /** We name these vertices: [0,..., size_fixed - 1]
     */
    int size_fixed;

    /**  Saves the neighbors of a vertex i \in [0,..., size_free - 1] in
     * neighbors_free[i]. These neighbors are sorted in ascending order.
     */
    std::vector<std::vector<int>> neighbors_free;

    /** Saves the neighbors of a fixed vertex i \in [0,..., size_fixed - 1] in
     * neighbors_fixed[i]. These neighbors are sorted in ascending order.
     */
    std::vector<std::vector<int>> neighbors_fixed;

    std::unordered_map<int, int> fixed_real_names;
    std::unordered_map<int, int> free_real_names;

    std::stack<std::tuple<int, int>> removed_vertices;

    CrossingMatrix crossing;

    PaceGraph(int a, int b, std::vector<std::tuple<int, int>> edges,
              std::unordered_map<int, int> fixed_real_names,
              std::unordered_map<int, int> free_real_names);

    /**
     *
     * @param a the number of size_fixed vertices
     * @param b the number of size_free vertices
     * @param edges an array of tuples (u, v) representing the edges of the
     * graph. u must be always in [0,..., a - 1] and v must be always in
     * [0,..., b - 1]
     */
    PaceGraph(int a, int b, std::vector<std::tuple<int, int>> edges)
        : PaceGraph(a, b, edges, createMap(a, 0), createMap(b, a)) {}

    /**
     * To read more about the .gr file format, see:
     * https://pacechallenge.org/2024/io/
     * @param gr a file stream (e.g. stdin or a file stream from a file)
     */
    static PaceGraph from_gr(std::istream &gr);

    static PaceGraph from_file(std::string filePath);

    void remove_free_vertices(std::vector<std::tuple<int, int, int>> vertices);

    PaceGraph induced_subgraphs(std::vector<int> fixed_nodes);

    std::string to_gr();

    std::string print_neighbors_fixed();

    int size() { return size_fixed + size_free; }
    int edge_count() const { return neighbors_free.size(); }

    std::tuple<std::vector<PaceGraph>, std::vector<int>> splitGraphOn0Splits();

    std::tuple<int, int> calculatingCrossingNumber(int u, int v);

    bool init_crossing_matrix_if_necessary();
};

#endif // PACE_GRAPH_HPP
