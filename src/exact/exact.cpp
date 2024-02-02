#include "exact.h"
#include "../pace_graph/pace_graph.hpp"
#include <stack>
#include <unordered_set>
using namespace std;

int decide(PaceGraph *g, int k) {
    if (g->edgeset.size() >= g->size_fixed + g->size_free + k)
        return 0;                          // Step 1
    tuple<int, int> intSysI[g->size_free]; // Interval system I
    for (int i = 0; i < g->size_free; i++) {
        intSysI[i] = make_tuple(g->size_fixed, -1);
    }
    for (int i = 0; i < g->edgeset.size();
         i++) { // initialize the first interval system
        tuple<int, int> it = g->edgeset[i];
        tuple<int, int> *p = &intSysI[get<1>(it) - g->size_fixed];
        if (get<0>(*p) > get<0>(it))
            get<0>(*p) = get<0>(it);
        if (get<1>(*p) < get<0>(it))
            get<1>(*p) = get<0>(it);
    }
    list<tuple<int, int, int>> orderedP;          // total order P
    tuple<int, int> intSysJ[g->size_free];        // Interval system J
    tuple<int, bool> intSysJM1[2 * g->size_free]; // Interval system J ^{-1}:
                                                  // true = left, false = right
    int cnt = 1; // values from 1 to 2 * g->size_free
    for (int i = 0; i < g->size_fixed; i++) {
        stack<int> pX1, pX2, pX3;
        for (int j = 0; j < g->neighbors2[i].size(); j++) {
            int y = g->neighbors2[i][j];
            if (g->neighbors2[y].size() == 1) {
                pX2.push(y);
            } // assumes deg(y) > 0
            else if (get<1>(intSysI[y - g->size_fixed]) == i)
                pX1.push(y);
            else if (get<0>(intSysI[y - g->size_fixed]) == i)
                pX3.push(y);
        }
        while (!pX1.empty()) {
            tuple<int, int> *p = &intSysJ[(pX1.top()) - g->size_fixed];
            intSysJM1[cnt] = make_tuple((pX1.top()) - g->size_fixed, false);
            get<1>(*p) = cnt++;
            pX1.pop();
        }
        while (!pX2.empty()) {
            tuple<int, int> *p = &intSysJ[(pX2.top()) - g->size_fixed];
            intSysJM1[cnt] = make_tuple((pX2.top()) - g->size_fixed, true);
            get<0>(*p) = cnt++;
            intSysJM1[cnt] = make_tuple((pX2.top()) - g->size_fixed, false);
            get<1>(*p) = cnt++;
            pX2.pop();
        }
        while (!pX3.empty()) {
            tuple<int, int> *p = &intSysJ[(pX3.top()) - g->size_fixed];
            intSysJM1[cnt] = make_tuple((pX3.top()) - g->size_fixed, true);
            get<0>(*p) = cnt++;
            pX3.pop();
        }
    }
    int c[g->size_free][g->size_free]; // array containing the crossing numbers
                                       // (only for orientable pairs)
    // we do not initialize the other pairs, and we will not use them
    unordered_set<int> yX; // {y \in Y | l_y < x < r_y}
    for (int i = 0; i < g->size_fixed;
         i++) { // STEP 1 -- initialize orientable pairs to 0
        for (int j = 0; j < g->neighbors2[i].size(); j++) {
            for (int l = 0; l < yX.size(); l++)
                c[j][l] = 0;
        }
        for (int j = 0; j < g->neighbors2[i].size(); j++) { // keep track of yX
            int y = g->neighbors2[i][j];
            if (get<0>(intSysI[y - g->size_fixed]) == i)
                yX.insert(y);
            if (get<1>(intSysI[y - g->size_fixed]) <= i + 1)
                yX.erase(y);
        }
    }
    int dX[g->size_free]; // d^{<x}(y)= |{z \in N(y) | z < x}|
    for (int i = 0; i < g->size_free; i++)
        dX[i] = 0;
    for (int i = 0; i < g->size_fixed; i++) { // STEP 2 -- fill the table
        for (int j = 0; j < g->neighbors2[i].size(); j++) {
            int u = g->neighbors2[i][j] - g->size_fixed;
            for (int l = 0; l < yX.size(); l++) {
                int v = l - g->size_fixed;
                c[u][v] += dX[v];
            }
        }
        for (int j = 0; j < g->neighbors2[i].size(); j++) {
            int u = g->neighbors2[i][j] - g->size_fixed;
            dX[u]++; // keep track of dX
            if (get<0>(intSysI[u]) == i)
                yX.insert(u); // keep track of yX
            if (get<1>(intSysI[u]) <= i + 1)
                yX.erase(u);
        }
        for (int j = 0; j < g->neighbors2[i].size(); j++) {
            int u = g->neighbors2[i][j] - g->size_fixed;
            for (int v = 0; v < yX.size(); v++) {
                if (get<1>(intSysI[u]) == i) {
                    c[v][u] += g->neighbors2[u].size() *
                               (g->neighbors2[v].size() - dX[v]);
                }
            }
        }
    }
    return 1;
}
int test() {
    PaceGraph p = PaceGraph::from_file("../data/website_20.gr");
    return decide(&p, 10);
}