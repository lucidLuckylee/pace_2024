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
    // Number of right vertices
    // Vertices in the right set are [left, left + right - 1]
    int right;

    // Vertices in the left set are [0, left - 1]
    int left;

    // TODO (Lukas, Fanny): Use both edgeset and neighbors list?
    std::vector<std::tuple<int, int>> edgeset;

    // Maps each vertex to their neighbors
    std::unordered_map<int, std::list<int>> neighbors;

    PaceGraph(int a, int b, std::vector<std::tuple<int, int>> edges);

    static PaceGraph from_gr(std::ifstream &gr);

    static PaceGraph from_file(std::string filePath);

    std::string to_gr();
};

#endif // PACE_GRAPH_HPP
