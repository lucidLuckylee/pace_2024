#include "pace_graph.hpp"

#include <cstdio>
#include <sstream>
#include <stdexcept>

// Implementation of member functions

PaceGraph::PaceGraph(int a, int b, std::vector<std::tuple<int, int>> edges) {
    size_A = a;
    size_B = b;

    edgeset = edges;

    for (const auto &edge : edges) {
        auto [head, tail] = edge;
        neighbors[head].push_back(tail);
        neighbors[tail].push_back(head);
    }
    for (int i = 0; i < neighbors.size(); i++) {
        std::list<int> nbs = neighbors[i];
        std::vector<int> adjVector;
        for (const auto &n : nbs)
            adjVector.push_back(n);
        neighbors2.push_back(adjVector);
    }

    crossing_matrix = new int[size_B * size_B];
    for (int i = 0; i < size_B * size_B; i++)
        crossing_matrix[i] = 0;

    for (int i = 0; i < size_B; i++) {
        for (int j = i + 1; j < size_B; j++) {

            for (int m : neighbors2[size_A + i]) {
                for (int n : neighbors2[size_A + j]) {
                    if (m > n) {
                        crossing_matrix[i * size_B + j]++;
                    } else if (n > m) {
                        crossing_matrix[j * size_B + i]++;
                    }
                }
            }
        }
    }
}

PaceGraph PaceGraph::from_gr(std::ifstream &gr) {
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
            edges.push_back(std::make_tuple(u - 1, v - 1));
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
    result << "p ocr " << size_A << " " << size_B << " " << edgeset.size()
           << "\n";

    for (const auto &edge : edgeset) {
        auto [head, tail] = edge;
        result << head + 1 << " " << tail + 1 << "\n";
    }

    return result.str();
}

void PaceGraph::print_crossing_matrix() {
    for (int i = 0; i < size_B; i++) {
        for (int j = 0; j < size_B; j++) {
            printf("%d ", crossing_matrix[i * size_B + j]);
        }
        printf("\n");
    }
}
