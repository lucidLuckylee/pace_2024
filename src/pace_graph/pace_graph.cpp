#include "pace_graph.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <stdexcept>

PaceGraph::PaceGraph(int a, int b, std::vector<std::tuple<int, int>> edges) {
    size_fixed = a;
    size_free = b;

    for (int i = 0; i < size_free; i++) {
        neighbors_free.emplace_back();
    }
    for (int i = 0; i < size_fixed; i++) {
        neighbors_fixed.emplace_back();
    }

    for (const auto &edge : edges) {
        auto [u, v] = edge;
        neighbors_fixed[u].push_back(v);
        neighbors_free[v].push_back(u);
    }

    for (auto &neighbors : neighbors_fixed) {
        std::sort(neighbors.begin(), neighbors.end());
    }

    for (auto &neighbors : neighbors_free) {
        std::sort(neighbors.begin(), neighbors.end());
    }

    crossing_matrix.resize(size_free);
    crossing_matrix_transposed.resize(size_free);
    for (int i = 0; i < size_free; i++) {
        crossing_matrix[i] = new int[size_free];
        crossing_matrix_transposed[i] = new int[size_free];

        for (int j = 0; j < size_free; j++) {
            crossing_matrix[i][j] = 0;
            crossing_matrix_transposed[i][j] = 0;
        }
    }

    for (int i = 0; i < size_free; i++) {
        for (int j = i + 1; j < size_free; j++) {
            for (int m : neighbors_free[i]) {
                for (int n : neighbors_free[j]) {
                    if (m > n) {
                        crossing_matrix[i][j]++;
                    } else if (n > m) {
                        crossing_matrix[j][i]++;
                    }
                }
            }
        }
    }

    for (int i = 0; i < size_free; ++i) {
        for (int j = 0; j < size_free; ++j) {
            crossing_matrix_transposed[i][j] = crossing_matrix[j][i];
        }
    }
}

PaceGraph PaceGraph::from_gr(std::istream &gr) {
    int a = 0;
    int b = 0;
    bool pfound = false;
    std::vector<std::tuple<int, int>> edges;

    std::string line;
    while (std::getline(gr, line)) {
        if (line[0] == 'p') {
            // TODO: Extend for parameterized track with cutwidth
            sscanf(line.c_str(), "p ocr %d %d", &a, &b);
            pfound = true;
        } else if (line[0] == 'c') {
            // Comment line, do nothing
        } else if (pfound) {
            int u, v;
            sscanf(line.c_str(), "%d %d", &u, &v);
            edges.push_back(std::make_tuple(u - 1, v - 1 - a));
        } else {
            throw std::invalid_argument(
                "ERROR: Encountered edge before p-line.");
        }
    }

    return PaceGraph(a, b, edges);
}

PaceGraph PaceGraph::from_file(std::string filePath) {
    std::ifstream grFile(filePath);
    // Check if the file is open
    if (!grFile.is_open()) {
        throw std::runtime_error("Error: Failed to open file.");
    }
    return PaceGraph::from_gr(grFile);
}

std::string PaceGraph::to_gr() {
    std::ostringstream result;

    int num_edges = 0;
    for (const auto &neighbors : neighbors_fixed) {
        num_edges += neighbors.size();
    }

    result << "p ocr " << size_fixed << " " << size_free << " " << num_edges
           << "\n";

    for (int i = 0; i < size_fixed; i++) {
        for (const auto &neighbor : neighbors_fixed[i]) {
            result << i + 1 << " " << neighbor + 1 << "\n";
        }
    }

    return result.str();
}

std::string PaceGraph::print_crossing_matrix() {
    std::ostringstream result;

    for (int i = 0; i < size_free; i++) {
        for (int j = 0; j < size_free; j++) {
            result << crossing_matrix[i][j] << " ";
        }
        result << std::endl;
    }
    return result.str();
}

PaceGraph::~PaceGraph() {
    for (int i = 0; i < size_free; i++) {
        delete[] crossing_matrix[i];
        delete[] crossing_matrix_transposed[i];
    }
}
