#ifndef PACE_GRAPH_HPP
#define PACE_GRAPH_HPP

#include "bool_crossing_matrix.hpp"
#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

struct DeleteInfo {
    int v;
    int position;
    long cost;

    DeleteInfo(int v, int position, long cost)
        : v(v), position(position), cost(cost){};
};

class PaceGraph {
  private:
    static std::vector<int> createMap(int limit, int offset) {
        std::vector<int> result(limit);
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

    std::vector<int> fixed_real_names;
    std::vector<int> free_real_names;
    
    // Saves the real name and position of a removed free vertex.
    std::stack<std::tuple<int, int>> removed_vertices;

    CrossingMatrix crossing;

    PaceGraph(int a, int b, std::vector<std::tuple<int, int>> &edges,
              std::vector<int> fixed_real_names,
              std::vector<int> free_real_names);

    /**
     *
     * @param a the number of size_fixed vertices
     * @param b the number of size_free vertices
     * @param edges an array of tuples (u, v) representing the edges of the
     * graph. u must be always in [0,..., a - 1] and v must be always in
     * [0,..., b - 1]
     */
    PaceGraph(int a, int b, std::vector<std::tuple<int, int>> &edges)
        : PaceGraph(a, b, edges, createMap(a, 0), createMap(b, a)) {}

    /**
     * To read more about the .gr file format, see:
     * https://pacechallenge.org/2024/io/
     * @param gr a file stream (e.g. stdin or a file stream from a file)
     */
    static PaceGraph from_gr(std::istream &gr);

    static PaceGraph from_file(std::string filePath);

    void remove_free_vertices(std::vector<DeleteInfo> vertices);

    std::unique_ptr<PaceGraph>
    induced_subgraphs_free(std::vector<int> fixed_nodes);

    std::unique_ptr<PaceGraph>
    induced_subgraphs_fixed(std::vector<int> fixed_nodes);

    std::string to_gr();

    std::string print_neighbors_fixed();

    int size() { return size_fixed + size_free; }
    int edge_count() const { return neighbors_free.size(); }

    std::tuple<std::vector<std::unique_ptr<PaceGraph>>, std::vector<int>>
    splitGraphs();
    static std::string
    split_graphs_to_gr(const std::vector<std::unique_ptr<PaceGraph>> &subgraphs,
                       const std::vector<int> &isolated_nodes,
                       const int original_size_fixed);

    std::tuple<int, int> calculatingCrossingNumber(int u, int v);

    bool init_crossing_matrix_if_necessary();
};

#endif // PACE_GRAPH_HPP
