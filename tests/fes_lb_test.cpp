#include "../src/exact/feedback_edge_set_solver.hpp"
#include "doctest.h"

FeedbackEdgeInstance getTestInstance() {
    std::vector<std::vector<std::shared_ptr<Edge>>> edges(
        6, std::vector<std::shared_ptr<Edge>>(6));

    edges[0][1] = std::make_shared<Edge>(0, 1, 1);
    edges[2][0] = std::make_shared<Edge>(2, 0, 1);
    edges[1][4] = std::make_shared<Edge>(1, 4, 1);
    edges[4][3] = std::make_shared<Edge>(4, 3, 1);
    edges[3][5] = std::make_shared<Edge>(3, 5, 1);
    edges[5][2] = std::make_shared<Edge>(5, 2, 1);
    edges[1][2] = std::make_shared<Edge>(1, 2, 5);
    edges[2][3] = std::make_shared<Edge>(2, 3, 5);
    edges[3][1] = std::make_shared<Edge>(3, 1, 5);

    std::vector<std::vector<int>> circlesId = {
        {1, 2, 3}, {0, 1, 2}, {1, 4, 3}, {2, 3, 5}};

    std::vector<std::shared_ptr<Circle>> circles;
    std::unordered_set<std::shared_ptr<Edge>> usedEdges;

    for (int i = 0; i < circlesId.size(); ++i) {
        circles.emplace_back(std::make_shared<Circle>());
        for (int j = 0; j < circlesId[i].size(); ++j) {
            int u = circlesId[i][j];
            int v = circlesId[i][(j + 1) % circlesId[i].size()];

            auto &edge = edges[u][v];
            circles[i]->edges.emplace_back(edge);
            edge->circles.emplace_back(circles[i]);
            usedEdges.insert(edge);
        }
        circles[i]->permuteEdges();
    }

    for (auto &edge : usedEdges) {
        edge->numberOfCircles = edge->circles.size();
        std::sort(edge->circles.begin(), edge->circles.end());
    }

    std::vector<int> globalSol = {0, 1, 2, 3, 4, 5};
    Order order(globalSol);

    FeedbackEdgeInstance instance(edges, circles, order, 0);
    for (auto &e : usedEdges) {
        instance.usedEdges.insert(e);
    }

    return instance;
}

TEST_CASE("Case where local search helps") {
    auto instance = getTestInstance();
    FeedbackEdgeSetSolver solver;

    long lb = solver.lbFeedbackEdgeSet(instance, 0, false);
    CHECK(lb == 5);
    long lbWithLocalSearch = solver.lbFeedbackEdgeSet(instance, 0, true);
    CHECK(lbWithLocalSearch == 7);
}

TEST_CASE("Case where edge is already selected") {
    FeedbackEdgeSetSolver solver;

    auto instance = getTestInstance();

    instance.edges[3][5]->selected = true;
    instance.edges[3][5]->potential = 10000;

    for (auto &c : instance.edges[3][5]->circles) {
        c->covered++;
    }

    long lb = solver.lbFeedbackEdgeSet(instance, 0, false);
    CHECK(lb == 5);

    long lbWithLocalSearch = solver.lbFeedbackEdgeSet(instance, 0, true);
    CHECK(lbWithLocalSearch == 6);
}
