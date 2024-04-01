//
// Created by jt on 24.03.24.
//

#ifndef PACE2024_DIRECTED_GRAPH_HPP
#define PACE2024_DIRECTED_GRAPH_HPP

#include "pace_graph.hpp"

#include <algorithm>
#include <bitset>
#include <iostream>
#include <set>
#include <stack>
#include <unordered_set>
#include <vector>

class DirectedGraph {
  private:
    std::vector<int> index, lowLink;
    std::vector<bool> onStack;
    std::stack<int> stack;
    std::vector<int> stack2;

    std::vector<bool> visited;
    std::vector<bool> blocked;
    std::vector<std::vector<int>> B;
    int currentIndex;

    void tarjansDFS(int v);
    void dfs(int v);

    bool findCycles(int s, int v, const std::vector<std::vector<int>> &adjList,
                    const std::vector<int> &indexToNode);
    void unblock(int node);

  public:
    std::vector<std::vector<int>> neighbors;
    std::vector<std::vector<int>> sccs;
    std::vector<std::bitset<20000>> reachabilityMatrix;
    std::vector<int> topologicalOrder;
    std::vector<std::vector<int>> cycles;

    explicit DirectedGraph(std::vector<std::vector<int>> neighbors);
    void init_sccs();
    DirectedGraph construct_sccs_graph();
    void init_reachability_matrix_dag();
    void topologicalSort();
    void findElementaryCycles();
};

class WeightedDirectedGraph : public DirectedGraph {

  public:
    explicit WeightedDirectedGraph(CrossingMatrix &crossingMatrix);
    std::vector<std::vector<int>> weights;
};

#endif // PACE2024_DIRECTED_GRAPH_HPP