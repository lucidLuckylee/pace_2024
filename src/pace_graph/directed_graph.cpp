//
// Created by jt on 24.03.24.
//

#include "directed_graph.hpp"

#include <bitset>
#include <queue>
#include <utility>

DirectedGraph::DirectedGraph(std::vector<std::vector<int>> &neighbors)
    : neighbors(std::move(neighbors)), visited(neighbors.size(), false),
      blocked(neighbors.size(), false), B(neighbors.size()),
      onStack(neighbors.size(), false) {
    for (auto &neighbor : neighbors) {
        std::sort(neighbor.begin(), neighbor.end());
    }
}

void DirectedGraph::init_sccs() {
    index.assign(neighbors.size(), -1);
    lowLink.resize(neighbors.size(), 0);
    onStack.assign(neighbors.size(), false);
    currentIndex = 0;
    sccs.clear();

    for (int i = 0; i < neighbors.size(); i++) {
        if (index[i] == -1) {
            tarjansDFS(i);
        }
    }

    index.clear();
    lowLink.clear();
    onStack.clear();
}
void DirectedGraph::tarjansDFS(int v) {
    index[v] = currentIndex;
    lowLink[v] = currentIndex;
    currentIndex++;
    stack.push(v);
    onStack[v] = true;

    for (int w : neighbors[v]) {
        if (index[w] == -1) {
            tarjansDFS(w);
            lowLink[v] = std::min(lowLink[v], lowLink[w]);
        } else if (onStack[w]) {
            lowLink[v] = std::min(lowLink[v], index[w]);
        }
    }

    if (lowLink[v] == index[v]) {
        std::vector<int> component;
        int w;
        do {
            w = stack.top();
            stack.pop();
            onStack[w] = false;
            component.push_back(w);
        } while (w != v);
        sccs.push_back(component);
    }
}
DirectedGraph DirectedGraph::construct_sccs_graph() {
    std::vector<std::vector<int>> sccs_neighbors(sccs.size(),
                                                 std::vector<int>());

    for (int i = 0; i < sccs.size(); i++) {

        auto &sccI = sccs[i];
        for (int j = 0; j < sccs.size(); j++) {

            if (i == j)
                continue;

            auto &sccJ = sccs[j];

            bool added = false;
            for (int u : sccI) {
                if (added)
                    break;
                for (int v : sccJ) {
                    if (std::binary_search(neighbors[u].begin(),
                                           neighbors[u].end(), v)) {
                        sccs_neighbors[i].push_back(j);
                        added = true;
                        break;
                    }
                }
            }
        }
    }

    return DirectedGraph(sccs_neighbors);
}
void DirectedGraph::init_reachability_matrix_dag() {

    reachabilityMatrix = std::vector<std::bitset<20000>>(neighbors.size());

    topologicalSort();
    std::reverse(topologicalOrder.begin(), topologicalOrder.end());

    for (const auto &u : topologicalOrder) {

        for (const auto &v : neighbors[u]) {
            reachabilityMatrix[u].set(v);
        }

        for (const auto &v : neighbors[u]) {
            reachabilityMatrix[u] |= reachabilityMatrix[v];
        }
    }
}
bool DirectedGraph::topologicalSort() {
    topologicalOrder.clear();
    std::fill(visited.begin(), visited.end(), false);
    std::fill(onStack.begin(), onStack.end(), false);

    for (int i = 0; i < neighbors.size(); ++i) {
        if (!visited[i]) {
            if (hasCycle(i)) {
                topologicalOrder.clear();
                return false;
            }
        }
    }

    std::reverse(topologicalOrder.begin(), topologicalOrder.end());
    return true;
}
bool DirectedGraph::hasCycle(int v) {
    visited[v] = true;
    onStack[v] = true;

    for (int w : neighbors[v]) {
        if (!visited[w]) {
            if (hasCycle(w)) {
                return true;
            }
        } else if (onStack[w]) {
            return true;
        }
    }

    onStack[v] = false;
    topologicalOrder.push_back(v);
    return false;
}

void DirectedGraph::findElementaryCycles() {

    cycles.clear();
    stack2.clear();
    std::fill(blocked.begin(), blocked.end(), false);
    std::fill(visited.begin(), visited.end(), false);

    for (auto &b : B) {
        b.clear();
    }
    init_sccs();

    std::vector<std::vector<int>> sccsStack = sccs;

    while (!sccsStack.empty()) {

        std::vector<int> scc = sccsStack.back();
        sccsStack.pop_back();

        if (scc.size() < 3)
            continue;

        stack2.clear();

        std::vector<std::vector<int>> sccAdjList(scc.size());
        std::unordered_map<int, int> nodeIndex;
        std::vector<int> indexToNode(scc.size());

        std::fill(blocked.begin(), blocked.end(), false);
        std::fill(visited.begin(), visited.end(), false);

        for (auto &b : B) {
            b.clear();
        }

        for (int i = 0; i < scc.size(); ++i) {
            nodeIndex[scc[i]] = i;
            indexToNode[i] = scc[i];
        }

        auto lowestNode = scc[0];
        for (int u : scc) {
            if (u < lowestNode) {
                lowestNode = u;
            }
            for (int v : neighbors[u]) {
                if (nodeIndex.count(v) > 0) {
                    sccAdjList[nodeIndex[u]].push_back(nodeIndex[v]);
                }
            }
        }
        int nodeInAdjList = nodeIndex[lowestNode];

        findCycles(nodeInAdjList, nodeInAdjList, sccAdjList, indexToNode);

        for (int i = 0; i < scc.size(); i++) {
            sccAdjList[i].erase(std::remove(sccAdjList[i].begin(),
                                            sccAdjList[i].end(), nodeInAdjList),
                                sccAdjList[i].end());
        }
        sccAdjList[nodeInAdjList].clear();
        DirectedGraph subGraph(sccAdjList);
        subGraph.init_sccs();
        for (auto &newScc : subGraph.sccs) {
            if (newScc.size() < 3)
                continue;
            std::vector<int> newSccNodes;
            for (int node : newScc) {
                newSccNodes.push_back(indexToNode[node]);
            }
            sccsStack.push_back(newSccNodes);
        }
    }
}

bool DirectedGraph::findCycles(int s, int v,
                               const std::vector<std::vector<int>> &adjList,
                               const std::vector<int> &indexToNode) {
    bool f = false;
    stack2.push_back(v);
    blocked[v] = true;

    for (const auto &w : adjList[v]) {
        if (w == s) {
            std::vector<int> cycle(stack2);
            cycles.push_back(cycle);
            f = true;
        } else if (!blocked[w]) {
            if (findCycles(s, w, adjList, indexToNode)) {
                f = true;
            }
        }
    }

    if (f) {
        unblock(v);
    } else {
        for (const auto &w : adjList[v]) {
            B[w].push_back(v);
        }
    }

    stack2.pop_back();
    return f;
}

void DirectedGraph::unblock(int node) {
    blocked[node] = false;
    for (int w : B[node]) {
        if (blocked[w]) {
            unblock(w);
        }
    }
    B[node].clear();
}

DirectedGraph DirectedGraph::from_pace_graph(PaceGraph &graph) {
    std::vector<std::vector<int>> neighbors(graph.size_free);
    for (int i = 0; i < graph.size_free; i++) {
        for (int j = i + 1; j < graph.size_free; j++) {
            auto [crossing_entries_i_j, crossing_entries_j_i] =
                graph.calculatingCrossingNumber(i, j);

            if (crossing_entries_i_j < crossing_entries_j_i) {
                neighbors[i].push_back(j);
            } else if (crossing_entries_i_j > crossing_entries_j_i) {
                neighbors[j].push_back(i);
            }
        }
    }
    return DirectedGraph(neighbors);
}

DirectedGraph DirectedGraph::dag_from_partial_order(CrossingMatrix &crossing) {
    std::vector<std::vector<int>> neighbors(crossing.matrix.size());

    for (int i = 0; i < crossing.matrix.size(); i++) {
        for (int j = 0; j < crossing.matrix.size(); j++) {
            if (crossing.lt(i, j)) {
                neighbors[i].push_back(j);
            }
        }
    }

    return DirectedGraph(neighbors);
}
std::vector<int> DirectedGraph::findShortestPath(int start, int end) {
    std::vector<int> path;
    std::vector<int> previous(neighbors.size(), -1);

    std::queue<int> q;
    q.push(start);
    previous[start] = start;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (u == end) {
            int current = end;
            while (current != start) {
                path.push_back(current);
                current = previous[current];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (int v : neighbors[u]) {
            if (previous[v] == -1) {
                previous[v] = u;
                q.push(v);
            }
        }
    }

    return {};
}

WeightedDirectedGraph::WeightedDirectedGraph(
    std::vector<std::vector<int>> &weights,
    std::vector<std::vector<int>> &neighbors)
    : DirectedGraph(neighbors), weights(weights) {}

WeightedDirectedGraph
WeightedDirectedGraph::from_matrix(CrossingMatrix &crossing) {
    std::vector<std::vector<int>> weights(
        crossing.matrix.size(), std::vector<int>(crossing.matrix.size(), 0));

    std::vector<std::vector<int>> neighbors(crossing.matrix.size());

    for (int u = 0; u < crossing.matrix.size(); ++u) {
        for (int v = 0; v < crossing.matrix.size(); ++v) {
            if (u == v)
                continue;
            if (crossing.matrix_diff[u][v] < 0) {
                weights[u][v] = -crossing.matrix_diff[u][v];
                neighbors[u].push_back(v);
            }
        }
    }
    return WeightedDirectedGraph(weights, neighbors);
}
