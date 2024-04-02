#include "pace_graph.hpp"
#include "directed_graph.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

PaceGraph::PaceGraph(int a, int b, std::vector<std::tuple<int, int>> &edges,
                     std::vector<int> fixed_real_names,
                     std::vector<int> free_real_names) {

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
            result << i + 1 << " " << neighbor + size_fixed + 1 << "\n";
        }
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

/*
 * Removes vertices from the free vertex set. Updates the crossing matrix,
 * neighbors_fixed and free_real_names, size_free, ub, lb and
 * cost_through_deleted_nodes. It also adds the vertex to a stack to later be
 * able to get the position of the vertex in the original graph.
 * @param vertices to be deleted described by the following tuples (v, position
 * to be inserted, cost)
 */
void PaceGraph::remove_free_vertices(
    std::vector<std::tuple<int, int, int>> vertices) {

    long costs = 0;
    std::sort(vertices.begin(), vertices.end(),
              [](auto a, auto b) { return std::get<1>(a) > std::get<1>(b); });

    for (const auto &[v, position, cost] : vertices) {
        removed_vertices.emplace(free_real_names[v], position);
        costs += cost;
    }

    std::vector<int> vertices_to_remove;
    for (const auto &vertex : vertices) {
        auto [v, _, _2] = vertex;
        vertices_to_remove.push_back(v);
    }

    std::sort(vertices_to_remove.begin(), vertices_to_remove.end());

    if (crossing.is_initialized()) {
        crossing.remove_free_vertices(vertices_to_remove);
    }

    // update vertices naming
    std::vector<int> mapping(size_free, -1);
    for (int i = 0, j = 0; i < size_free; i++) {
        if (j < vertices_to_remove.size() && i == vertices_to_remove[j]) {
            j++;
        } else {
            mapping[i] = i - j;
            free_real_names[i - j] = free_real_names[i];
        }
    }

    for (int i = vertices_to_remove.size() - 1; i >= 0; i--) {
        neighbors_free.erase(neighbors_free.begin() + vertices_to_remove[i]);
    }

    // update edges
    for (auto &edge_list : neighbors_fixed) {
        edge_list.erase(
            std::remove_if(edge_list.begin(), edge_list.end(),
                           [&mapping](int v) { return mapping[v] == -1; }),
            edge_list.end());

        for (int &v : edge_list) {
            v = mapping[v];
        }
    }

    size_free -= vertices_to_remove.size();
    cost_through_deleted_nodes += costs;
    ub -= costs;
    lb = std::max(0L, lb - costs);
}

std::tuple<std::vector<std::unique_ptr<PaceGraph>>, std::vector<int>>
PaceGraph::splitGraphs() {
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

    std::vector<std::unique_ptr<PaceGraph>> graphs;
    for (const std::vector<int> &induced_vertices : result) {
        auto g = induced_subgraphs_fixed(induced_vertices);
        if (g->init_crossing_matrix_if_necessary()) {
            auto dg = DirectedGraph::from_pace_graph(*g);
            dg.init_sccs();

            if (dg.sccs.size() == 1) {
                graphs.push_back(std::move(g));
                continue;
            }

            auto sccGraph = dg.construct_sccs_graph();
            sccGraph.topologicalSort();
            std::reverse(sccGraph.topologicalOrder.begin(),
                         sccGraph.topologicalOrder.end());

            for (int i = 0; i < sccGraph.topologicalOrder.size(); ++i) {
                for (int j = i + 1; j < sccGraph.topologicalOrder.size(); ++j) {
                    for (int u : dg.sccs[sccGraph.topologicalOrder[i]]) {
                        for (int v : dg.sccs[sccGraph.topologicalOrder[j]]) {
                            cost_through_deleted_nodes +=
                                g->crossing.matrix[u][v];
                        }
                    }
                }
            }

            for (const auto &scc : sccGraph.topologicalOrder) {

                auto &free_vertices = dg.sccs[scc];
                auto induced = g->induced_subgraphs_free(free_vertices);
                graphs.push_back(std::move(induced));
            }

            g->crossing.clean();
        } else {
            graphs.push_back(std::move(g));
        }
    }

    return std::make_tuple(std::move(graphs), std::move(isolatedNodes));
}

std::unique_ptr<PaceGraph>
PaceGraph::induced_subgraphs_free(std::vector<int> free_nodes) {
    std::vector<bool> is_used = std::vector<bool>(size_fixed, false);
    for (const auto &v : free_nodes) {
        for (const auto &u : neighbors_free[v]) {
            is_used[u] = true;
        }
    }

    std::vector<int> old_fixed_to_new_fixed(size_fixed, -1);
    std::vector<int> new_fixed_real_names;

    int fixedCount = 0;
    for (int i = 0; i < size_fixed; ++i) {
        if (is_used[i]) {
            new_fixed_real_names.push_back(fixed_real_names[i]);
            old_fixed_to_new_fixed[i] = fixedCount;
            fixedCount++;
        }
    }

    std::vector<int> new_free_real_names(free_nodes.size());
    std::vector<std::tuple<int, int>> edges;

    for (int i = 0; i < free_nodes.size(); i++) {
        int v = free_nodes[i];
        new_free_real_names[i] = free_real_names[v];
        for (int u : neighbors_free[v]) {
            int new_u = old_fixed_to_new_fixed[u];
            edges.emplace_back(new_u, i);
        }
    }

    return std::make_unique<PaceGraph>(fixedCount, free_nodes.size(), edges,
                                       new_fixed_real_names,
                                       new_free_real_names);
}

std::unique_ptr<PaceGraph>
PaceGraph::induced_subgraphs_fixed(std::vector<int> fixed_nodes) {

    std::vector<bool> is_used = std::vector<bool>(size_free, false);
    for (const auto &v : fixed_nodes) {
        for (const auto &u : neighbors_fixed[v]) {
            is_used[u] = true;
        }
    }

    std::vector<int> new_free_real_names;
    std::vector<int> old_free_to_new_free(size_free, -1);

    int freeCount = 0;
    for (int i = 0; i < size_free; ++i) {
        if (is_used[i]) {
            new_free_real_names.push_back(free_real_names[i]);
            old_free_to_new_free[i] = freeCount;
            freeCount++;
        }
    }

    std::vector<int> new_fixed_real_names(fixed_nodes.size());
    std::vector<std::tuple<int, int>> edges;

    for (int i = 0; i < fixed_nodes.size(); i++) {
        int u = fixed_nodes[i];
        new_fixed_real_names[i] = fixed_real_names[u];

        for (int v : neighbors_fixed[u]) {

            int new_v = old_free_to_new_free[v];

            edges.emplace_back(i, new_v);
        }
    }

    return std::make_unique<PaceGraph>(fixed_nodes.size(), freeCount, edges,
                                       new_fixed_real_names,
                                       new_free_real_names);
}

std::tuple<int, int> PaceGraph::calculatingCrossingNumber(int u, int v) {

    if (crossing.is_initialized()) {
        return std::make_tuple(crossing.matrix[u][v], crossing.matrix[v][u]);
    }

    int crossing_entries_u_v = 0;
    int crossing_entries_v_u = 0;

    int currentVPointer = 0;
    int currentUPointer = 0;

    const auto &u_neighbors = neighbors_free[u];
    const auto &v_neighbors = neighbors_free[v];

    for (int u_N : u_neighbors) {
        while (currentVPointer < v_neighbors.size() &&
               v_neighbors[currentVPointer] < u_N) {
            crossing_entries_u_v += u_neighbors.size() - currentUPointer;
            currentVPointer++;
        }
        crossing_entries_v_u += v_neighbors.size() - currentVPointer;
        currentUPointer++;
    }

    while (currentVPointer < v_neighbors.size()) {
        crossing_entries_u_v += u_neighbors.size() - currentUPointer;
        currentVPointer++;
    }

    return std::make_tuple(crossing_entries_u_v, crossing_entries_v_u);
}
bool PaceGraph::init_crossing_matrix_if_necessary() {
    if (!crossing.is_initialized()) {
        if (!crossing.can_initialized(*this)) {
            return false;
        }
        crossing.init_crossing_matrix(*this);
        return true;
    }
    return true;
}
