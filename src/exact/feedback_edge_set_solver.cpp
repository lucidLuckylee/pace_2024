
#include "feedback_edge_set_solver.hpp"
#include "../heuristic_solver/genetic_algorithm.hpp"

void Circle::permuteEdges() {
    if (!edges.empty()) {
        auto minIt = std::min_element(
            edges.begin(), edges.end(),
            [](const std::shared_ptr<Edge> &a, const std::shared_ptr<Edge> &b) {
                return a->start < b->start;
            });
        std::rotate(edges.begin(), minIt, edges.end());
    }
}

Order FeedbackEdgeSetSolver::run(PaceGraph &graph) {

    graph.init_crossing_matrix_if_necessary();

    WeightedDirectedGraph weightedDirectedGraph =
        WeightedDirectedGraph::from_matrix(graph.crossing);

    if (weightedDirectedGraph.topologicalSort()) {
        return Order(weightedDirectedGraph.topologicalOrder);
    }

    FeedbackEdgeInstance instance(weightedDirectedGraph);

    GeneticHeuristicParameter geneticHeuristicParameter;
    GeneticHeuristic geneticHeuristic([](auto it) { return it <= 2000; },
                                      geneticHeuristicParameter);

    solveFeedbackEdgeSet(instance);

    Order goodOrder = geneticHeuristic.solve(graph);
    auto fesInit = approximateFeedbackEdgeSet(weightedDirectedGraph,
                                              instance.edges, goodOrder);
    addCycleMatrixElements(weightedDirectedGraph, fesInit, instance);

    int iter = 0;
    while (true) {
        iter++;
        solveFeedbackEdgeSet(instance);

        std::vector<std::vector<int>> new_neighbours(
            weightedDirectedGraph.neighbors.begin(),
            weightedDirectedGraph.neighbors.end());

        for (auto &edge : instance.bestSolution) {
            auto it =
                std::lower_bound(new_neighbours[edge->start].begin(),
                                 new_neighbours[edge->start].end(), edge->end);

            if (it != new_neighbours[edge->start].end() && *it == edge->end) {
                new_neighbours[edge->start].erase(it);
            }
        }

        DirectedGraph G_i = DirectedGraph(new_neighbours);
        if (G_i.topologicalSort()) {
            std::cerr << "# Iterations: " << iter << std::endl;
            return Order(G_i.topologicalOrder);
        }

        auto fas = approximateFeedbackEdgeSet(G_i, instance.edges, goodOrder);
        addCycleMatrixElements(G_i, fas, instance);
    }
}

void FeedbackEdgeSetSolver::solveFeedbackEdgeSet(FeedbackEdgeInstance &instance,
                                                 int k) {

    if (k + lbFeedbackEdgeSet(instance) >= instance.ub) {
        return;
    }

    int cycleId = -1;
    for (int i = 0; i < instance.circles.size(); i++) {
        if (instance.circles[i]->covered == 0) {
            cycleId = i;
            break;
        }
    }

    if (cycleId == -1) {
        std::vector<std::shared_ptr<Edge>> solution;
        int weight = 0;
        for (int i = 0; i < instance.edges.size(); i++) {
            for (int j = 0; j < instance.edges.size(); j++) {
                if (instance.edges[i][j]->selected) {
                    weight += instance.edges[i][j]->weight;
                    solution.emplace_back(instance.edges[i][j]);
                }
            }
        }

        if (weight < instance.ub) {
            instance.ub = weight;
            instance.bestSolution = solution;
        }
        return;
    }

    auto &cycle = instance.circles[cycleId];

    for (auto &edge : cycle->edges) {
        edge->selected = true;
        for (auto &circle : edge->circles) {
            circle->covered++;
        }

        solveFeedbackEdgeSet(instance, k + edge->weight);
        edge->selected = false;
        for (auto &circle : edge->circles) {
            circle->covered--;
        }
    }
}

void FeedbackEdgeSetSolver::solveFeedbackEdgeSet(
    FeedbackEdgeInstance &instance) {
    instance.ub = 0;
    instance.bestSolution.clear();
    approximateFeedbackEdgeSet(instance);
    std::cerr << "UB: " << instance.ub;
    findGoodCircleOrderForLB(instance);
    solveFeedbackEdgeSet(instance, 0);
    std::cerr << " LB: " << lbFeedbackEdgeSet(instance);

    std::cerr << " Exact: " << instance.ub << std::endl;
}

void FeedbackEdgeSetSolver::addCycleMatrixElements(
    DirectedGraph &graph, std::vector<std::shared_ptr<Edge>> &feedbackEdgeSet,
    FeedbackEdgeInstance &instance) {
    for (auto &edge : feedbackEdgeSet) {
        auto u = edge->start;
        auto v = edge->end;

        auto new_circle = graph.findShortestPath(v, u);
        if (new_circle.empty()) {
            continue;
        }

        Circle circle;
        for (int i = 0; i < new_circle.size(); ++i) {
            int u = new_circle[i];
            int v = new_circle[(i + 1) % new_circle.size()];
            auto &e = instance.edges[u][v];

            circle.edges.emplace_back(e);
        }

        circle.permuteEdges();
        if (!instance.containCircle(circle)) {

            auto c = std::make_shared<Circle>(circle);
            instance.circles.emplace_back(c);
            for (auto &e : c->edges) {
                e->circles.emplace_back(c);
                e->numberOfCircles++;
                instance.usedEdges.insert(e);
            }
        }
    }
}

std::vector<std::shared_ptr<Edge>>
FeedbackEdgeSetSolver::approximateFeedbackEdgeSet(
    DirectedGraph &graph,
    std::vector<std::vector<std::shared_ptr<Edge>>> &edges, Order &order) {

    std::vector<std::shared_ptr<Edge>> feedbackEdgeSet;
    for (int i = 0; i < graph.neighbors.size(); i++) {
        for (int j = 0; j < i - 1; j++) {
            auto &edge = edges[order.get_vertex(i)][order.get_vertex(j)];
            if (edge->weight == 0) {
                continue;
            }
            feedbackEdgeSet.emplace_back(edge);
        }
    }
    return feedbackEdgeSet;
}

void FeedbackEdgeSetSolver::approximateFeedbackEdgeSet(
    FeedbackEdgeInstance &instance) {
    std::shared_ptr<Edge> edge = nullptr;
    double bestCost = 100000000;
    for (auto &e : instance.usedEdges) {
        if (e->numberOfCircles > 0 && !e->selected) {
            double cost = static_cast<double>(e->weight) / e->numberOfCircles;
            if (cost < bestCost) {
                bestCost = cost;
                edge = e;
            }
        }
    }

    if (edge == nullptr) {
        for (auto &edge : instance.usedEdges) {
            if (edge->selected) {
                instance.ub += edge->weight;
                instance.bestSolution.emplace_back(edge);
            }
        }

        return;
    }

    edge->selected = true;
    for (auto &c : edge->circles) {
        c->covered++;
        for (auto &e : c->edges) {
            e->numberOfCircles--;
        }
    }

    approximateFeedbackEdgeSet(instance);

    edge->selected = false;
    for (auto &c : edge->circles) {
        c->covered--;
        for (auto &e : c->edges) {
            e->numberOfCircles++;
        }
    }
}

long FeedbackEdgeSetSolver::lbFeedbackEdgeSet(FeedbackEdgeInstance &instance) {
    for (auto &e : instance.usedEdges) {
        if (!e->selected) {
            e->potential = e->weight;
        }
    }

    long lb = 0;
    for (auto &c : instance.circles) {
        if (c->covered == 0) {
            int min = c->edges[0]->potential;
            for (auto &e : c->edges) {
                if (e->potential < min) {
                    min = e->potential;
                }
            }

            lb += min;
            for (auto &e : c->edges) {
                e->potential -= min;
            }
        }
    }

    return lb;
}

void FeedbackEdgeSetSolver::findGoodCircleOrderForLB(
    FeedbackEdgeInstance &instance) {
    long bestLB = lbFeedbackEdgeSet(instance);
    std::vector<std::shared_ptr<Circle>> bestCircles = instance.circles;
    for (int i = 0; i < 100; i++) {

        unsigned seed =
            std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(instance.circles.begin(), instance.circles.end(),
                     std::default_random_engine(seed));

        long lb = lbFeedbackEdgeSet(instance);

        if (lb > bestLB) {
            bestLB = lb;
            bestCircles = instance.circles;
        }
    }
    instance.circles = std::move(bestCircles);
}

bool FeedbackEdgeInstance::containCircle(Circle &circle) {
    return std::any_of(circles.begin(), circles.end(),
                       [&circle](const std::shared_ptr<Circle> &c) {
                           return c->edges == circle.edges;
                       });
}
