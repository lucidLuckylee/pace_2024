#include "exact.h"
#include "../pace_graph/pace_graph.hpp"
#include <map>
#include <stack>
#include <tuple>
#include <unordered_set>

#include <ranges>
#include <vector>

int decide(PaceGraph *g, int k) {

    if (g->edge_count() >= g->size() + k)
        return 0; // Step 1

    std::tuple<int, int> intSysI[g->size_free]; // Interval system I

    for (auto i = 0; i < g->size_free; i++){
        intSysI[i] = std::make_tuple(g->size_fixed, -1);
    }

    //c++20 (iota)
    // for (auto i : std::ranges::views::iota(0, g->size_free)) {
    //     intSysI[i] = std::make_tuple(g->size_fixed, -1);
    // }

    for (auto vertex = 0; vertex < g->neighbors_free.size(); vertex++){
        for (auto neigh : g->neighbors_free[vertex]) {
            if (std::get<0>(intSysI[vertex]) > neigh)
                std::get<0>(intSysI[vertex]) = neigh;
            if (std::get<1>(intSysI[vertex]) < neigh)
                std::get<1>(intSysI[vertex]) = neigh;
        }
    }

    //c++23 (enumerate)
    // for (auto [v, neighbors] :
    //      std::ranges::views::enumerate(g->neighbors_free)) {
    //     for (auto neigh : neighbors) {
    //         if (std::get<0>(intSysI[v]) > neigh)
    //             std::get<0>(intSysI[v]) = neigh;
    //         if (std::get<1>(intSysI[v]) < neigh)
    //             std::get<1>(intSysI[v]) = neigh;
    //     }
    // }

    const int SIZE_INT_SYS_J = 2 * g->size_free;

    std::list<std::tuple<int, int, int>> orderedP; // total order P
    std::tuple<int, int> intSysJ[g->size_free];    // Interval system J
    std::tuple<int, bool>
        intSysJM1[2 * g->size_free]; // Interval system J ^{-1}:
                                     // true = left, false = right
    int cnt = 1;                     // values from 1 to 2 * g->size_free
    for (int i = 0; i < g->size_fixed; i++) {
        std::stack<int> pX1, pX2, pX3;
        for (int j = 0; j < g->neighbors_free[i].size(); j++) {
            int y = g->neighbors_free[i][j];
            if (g->neighbors_free[y].size() == 1) {
                pX2.push(y);
            } // assumes deg(y) > 0
            else if (std::get<1>(intSysI[y - g->size_fixed]) == i)
                pX1.push(y);
            else if (std::get<0>(intSysI[y - g->size_fixed]) == i)
                pX3.push(y);
        }
        while (!pX1.empty()) {
            std::tuple<int, int> *p = &intSysJ[(pX1.top()) - g->size_fixed];
            intSysJM1[cnt] =
                std::make_tuple((pX1.top()) - g->size_fixed, false);
            std::get<1>(*p) = cnt++;
            pX1.pop();
        }
        while (!pX2.empty()) {
            std::tuple<int, int> *p = &intSysJ[(pX2.top()) - g->size_fixed];
            intSysJM1[cnt] = std::make_tuple((pX2.top()) - g->size_fixed, true);
            std::get<0>(*p) = cnt++;
            intSysJM1[cnt] =
                std::make_tuple((pX2.top()) - g->size_fixed, false);
            std::get<1>(*p) = cnt++;
            pX2.pop();
        }
        while (!pX3.empty()) {
            std::tuple<int, int> *p = &intSysJ[(pX3.top()) - g->size_fixed];
            intSysJM1[cnt] = std::make_tuple((pX3.top()) - g->size_fixed, true);
            std::get<0>(*p) = cnt++;
            pX3.pop();
        }
    }
    int c[g->size_free][g->size_free]; // array containing the crossing numbers
                                       // (only for orientable pairs)
    // we do not initialize the other pairs, and we will not use them
    std::unordered_set<int> yX; // {y \in Y | l_y < x < r_y}
    for (int i = 0; i < g->size_fixed;
         i++) { // STEP 1 -- initialize orientable pairs to 0
        for (int j = 0; j < g->neighbors_free[i].size(); j++) {
            for (int l = 0; l < yX.size(); l++)
                c[j][l] = 0;
        }
        for (int j = 0; j < g->neighbors_free[i].size();
             j++) { // keep track of yX
            int y = g->neighbors_free[i][j];
            if (std::get<0>(intSysI[y - g->size_fixed]) == i)
                yX.insert(y);
            if (std::get<1>(intSysI[y - g->size_fixed]) <= i + 1)
                yX.erase(y);
        }
    }
    int dX[g->size_free]; // d^{<x}(y)= |{z \in N(y) | z < x}|
    for (int i = 0; i < g->size_free; i++)
        dX[i] = 0;
    for (int i = 0; i < g->size_fixed; i++) { // STEP 2 -- fill the table
        for (int j = 0; j < g->neighbors_free[i].size(); j++) {
            int u = g->neighbors_free[i][j] - g->size_fixed;
            for (int l = 0; l < yX.size(); l++) {
                int v = l - g->size_fixed;
                c[u][v] += dX[v];
            }
        }
        for (int j = 0; j < g->neighbors_free[i].size(); j++) {
            int u = g->neighbors_free[i][j] - g->size_fixed;
            dX[u]++; // keep track of dX
            if (std::get<0>(intSysI[u]) == i)
                yX.insert(u); // keep track of yX
            if (std::get<1>(intSysI[u]) <= i + 1)
                yX.erase(u);
        }
        for (int j = 0; j < g->neighbors_free[i].size(); j++) {
            int u = g->neighbors_free[i][j] - g->size_fixed;
            for (int v = 0; v < yX.size(); v++) {
                if (std::get<1>(intSysI[u]) == i) {
                    c[v][u] += g->neighbors_free[u].size() *
                               (g->neighbors_free[v].size() - dX[v]);
                }
            }
        }
    }

    std::unordered_set<int> leftT[SIZE_INT_SYS_J];
    std::unordered_set<int> middleT[SIZE_INT_SYS_J];
    std::unordered_set<int> rightT[SIZE_INT_SYS_J];

    leftT[0] = std::unordered_set<int>{};
    middleT[0] = std::unordered_set<int>{};
    rightT[0] = std::unordered_set<int>{};
    

    auto comput_sets_at_t = [&leftT, &middleT, &rightT, &intSysJ](int t){
        int vertex = 0;
        for (auto [ay, by]: intSysJ){
            if (ay <= t) {
                leftT[t].insert(vertex);
            } else if (ay <= t < by){
                middleT[t].insert(vertex);
            } else if (t < ay){
                rightT[t].insert(vertex);
            }
            vertex +=1;
        }
    };
    
    auto compute_cost = [&leftT](auto &c, int t, int vertex){
        int cost = 0;
        for(auto vertex_L : leftT[t]){
            cost = cost + c[vertex][vertex_L];
        }
        return cost;
    };

    std::map<std::tuple<int, int>, int>
        cost; // leftT, {y} |-> cost where y \in middleT
    std::map<std::unordered_set<int>, int> opt;

    for (int t : std::ranges::views::iota(0, SIZE_INT_SYS_J)) {
       /* move over t -> compute cost table and calculate opt_t*/
       comput_sets_at_t(t);
       for(auto y : middleT[t]){
        cost.insert(leftT[t], compute_cost(c, t, y));
       }
    }
    return 1;
}
int test() {
    PaceGraph p = PaceGraph::from_file("../data/website_20.gr");
    return decide(&p, 10);
}