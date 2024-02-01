#ifndef PACE_GRAPH_HPP
#define PACE_GRAPH_HPP

#include <fstream>
#include <list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace std {}
class PaceGraph {
  public:
    // Number of size_B vertices
    // Vertices in the size_B set are [size_A, size_A + size_B - 1]
    int size_B;

    // Vertices in the size_A set are [0, size_A - 1]
    int size_A;

    // TODO (Lukas, Fanny): Use both edgeset and neighbors list?
    std::vector<std::tuple<int, int>> edgeset;

    // Maps each vertex to their neighbors
    std::unordered_map<int, std::list<int>> neighbors;

    // maybe sort it?
    std::vector<std::vector<int>> neighbors2;

    // size: size_B x size_B
    // crossing_matrix[i*size_B + j] = number of crossings when i is placed
    // before j
    int *crossing_matrix;

    PaceGraph(int a, int b, std::vector<std::tuple<int, int>> edges);

    static PaceGraph from_gr(std::ifstream &gr);

    static PaceGraph from_file(std::string filePath);

    std::string to_gr();
    void print_crossing_matrix();
};

#endif // PACE_GRAPH_HPP
