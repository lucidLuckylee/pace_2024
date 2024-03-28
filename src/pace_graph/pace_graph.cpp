#include "pace_graph.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

PaceGraph::PaceGraph(int a, int b, std::vector<std::tuple<int, int>> edges,
                     std::unordered_map<int, int> fixed_real_names,
                     std::unordered_map<int, int> free_real_names) {

    size_fixed = a;
    size_free = b;

    this->fixed_real_names = std::move(fixed_real_names);
    this->free_real_names = std::move(free_real_names);

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
        } else if (line.empty()) {
            // Empty line, do nothing
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
            result << fixed_real_names[i] << " " << free_real_names[neighbor]
                   << "\n";
        }
    }

    return result.str();
}

std::string PaceGraph::print_crossing_matrix() {
    std::ostringstream result;

    if (!is_crossing_matrix_initialized()) {
        return "Not initialized";
    }

    for (int i = 0; i < crossing_matrix.size(); i++) {
        for (int j = 0; j < size_free; j++) {
            result << crossing_matrix[i][j] << " ";
        }
        result << std::endl;
    }
    return result.str();
}

std::string PaceGraph::print_neighbors_fixed() {
    std::ostringstream result;

    for (int i = 0; i < neighbors_fixed.size(); i++) {
        result << "Fixed vertex {" << i << "} neighbors: " << std::endl;
        for (auto neighbor : neighbors_fixed[i]) {
            result << neighbor << ", ";
        }
        result << std::endl;
    }
    return result.str();
}

bool PaceGraph::is_crossing_matrix_initialized() {
    return !crossing_matrix.empty();
}

bool PaceGraph::init_crossing_matrix_if_necessary() {

    if (is_crossing_matrix_initialized()) {
        return true;
    }

    if (size_free > 20000) {
        return false;
    }

    crossing_matrix.resize(size_free);
    for (int i = 0; i < size_free; i++) {
        crossing_matrix[i] = new int[size_free];

        for (int j = 0; j < size_free; j++) {
            crossing_matrix[i][j] = 0;
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

    // TODO: Split this call out (or create a new different init_cross_matrix
    // without it) so we only call it once after doing all the data reductions
    init_crossing_matrix_diff();

    return true;
}

/*
 * Reserves memory for and initializes the crossing_matrix_diff field from the
 * crossing_matrix. Does nothing when crossing_matrix is not initialized.
 */
void PaceGraph::init_crossing_matrix_diff() {
    if (is_crossing_matrix_initialized()) {
        // Reserve memory
        crossing_matrix_diff.resize(size_free);
        for (int i = 0; i < size_free; i++) {
            crossing_matrix_diff[i] = new int[size_free];
            for (int j = 0; j < size_free; j++) {
                crossing_matrix_diff[i][j] = 0;
            }
        }

        // Calculate entries
        for (int i = 0; i < size_free; ++i) {
            for (int j = 0; j < size_free; ++j) {
                crossing_matrix_diff[i][j] =
                    crossing_matrix[i][j] - crossing_matrix[j][i];
            }
        }
    }
}

/*
 * Removes a free vertex v from the free vertex set and sets size_free to
 * size_free - 1. Updates the crossing_matrix, neighbors_fixed and
 * free_real_names but NOT crossing_matrix_diff!
 */
void PaceGraph::remove_free_vertex(int v) {
    // TODO: Store v so we can later put it back in the order when printing it
    // out

    // Drop crossing_matrix[v]
    int *v_entries = crossing_matrix[v];
    crossing_matrix.erase(crossing_matrix.begin() + v);
    delete[] v_entries;

    // Adjust real names and crossing_matrix entries that come after v
    for (int i = v; i < size_free - 1; i++) {
        free_real_names[i] = free_real_names[i + 1];
        for (int u = 0; u < crossing_matrix.size(); u++) {
            // Move all entries after v up one index
            crossing_matrix[u][i] = crossing_matrix[u][i + 1];
        }
    }
    // NOTE: We do not delete the last cell of crossing_matrix[u] so we are not
    // freeing up memory yet other than crossing_matrix[v].

    // Shrink free_real_names
    free_real_names.erase(size_free - 1);

    // Remove v from neighbors_fixed[u] for all u in neighbors_fixed and adjust
    // all neighbors w with w > v.
    for (int u = 0; u < neighbors_fixed.size(); u++) {
        neighbors_fixed[u].erase(std::remove(neighbors_fixed[u].begin(),
                                             neighbors_fixed[u].end(), v),
                                 neighbors_fixed[u].end());
        for (int i = 0; i < neighbors_fixed[u].size(); i++) {
            if (neighbors_fixed[u][i] > v) {
                neighbors_fixed[u][i] -= 1;
            }
        }
    }

    // Remove neighbors_free[v]
    neighbors_free.erase(neighbors_free.begin() + v);
    size_free = size_free - 1;
}

PaceGraph::~PaceGraph() {
    for (int i = 0; i < crossing_matrix.size(); i++) {
        delete[] crossing_matrix[i];
        delete[] crossing_matrix_diff[i];
    }
}

std::tuple<std::vector<PaceGraph>, std::vector<int>>
PaceGraph::splitGraphOn0Splits() {
    long current_count = 0;

    std::vector<std::vector<int>> result;

    result.emplace_back();

    std::vector<int> isolatedNodes;
    for (int v = 0; v < size_free; v++) {
        if (neighbors_free[v].empty()) {
            isolatedNodes.push_back(v);
        }
    }
    result.back().push_back(0);

    for (int u = 0; u < size_fixed - 1; ++u) {
        auto neighbors = neighbors_fixed[u];

        for (int v : neighbors) {
            auto neighbors_v = neighbors_free[v];
            for (int w : neighbors_v) {
                if (u > w) {
                    current_count--;
                } else if (u < w) {
                    current_count++;
                }
            }
        }

        if (current_count == 0) {
            result.emplace_back();
        }

        result.back().push_back(u + 1);
    }

    for (std::vector<int> &induced_verticies : result) {
        induced_verticies.erase(
            std::remove_if(
                induced_verticies.begin(), induced_verticies.end(),
                [this](int x) { return neighbors_fixed[x].empty(); }),
            induced_verticies.end());
    }

    result.erase(std::remove_if(result.begin(), result.end(),
                                [](std::vector<int> x) { return x.empty(); }),
                 result.end());

    std::vector<PaceGraph> graphs;
    for (const std::vector<int> &induced_vertices : result) {
        graphs.push_back(induced_subgraphs(induced_vertices));
    }

    return std::tuple<std::vector<PaceGraph>, std::vector<int>>(graphs,
                                                                isolatedNodes);
}
PaceGraph PaceGraph::induced_subgraphs(std::vector<int> fixed_nodes) {

    std::set<int> nodesInB;
    std::unordered_map<int, int> new_fixed_real_names;
    std::unordered_map<int, int> new_free_real_names;

    std::unordered_map<int, int> old_free_to_new_free;

    std::vector<std::tuple<int, int>> edges;

    int freeCount = 0;
    for (int i = 0; i < fixed_nodes.size(); i++) {
        int u = fixed_nodes[i];
        new_fixed_real_names[new_fixed_real_names.size()] = fixed_real_names[u];

        for (int v : neighbors_fixed[u]) {

            if (old_free_to_new_free.find(v) == new_free_real_names.end()) {
                new_free_real_names[freeCount] = free_real_names[v];

                old_free_to_new_free[v] = freeCount;
                freeCount++;
            }

            int new_v = old_free_to_new_free[v];

            edges.emplace_back(i, new_v);

            nodesInB.insert(v);
        }
    }

    return PaceGraph(fixed_nodes.size(), nodesInB.size(), edges,
                     new_fixed_real_names, new_free_real_names);
}
void PaceGraph::fixNodeOrder(int beforeNode, int afterNode) {
    crossing_matrix[afterNode][beforeNode] += INF;
    crossing_matrix_diff[afterNode][beforeNode] += INF;
    crossing_matrix_diff[beforeNode][afterNode] -= INF;
}
void PaceGraph::unfixNodeOrder(int beforeNode, int afterNode) {
    crossing_matrix[afterNode][beforeNode] -= INF;
    crossing_matrix_diff[afterNode][beforeNode] -= INF;
    crossing_matrix_diff[beforeNode][afterNode] += INF;
}
