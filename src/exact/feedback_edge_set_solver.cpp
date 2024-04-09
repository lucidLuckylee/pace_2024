
#include "feedback_edge_set_solver.hpp"
#include "../heuristic_solver/genetic_algorithm.hpp"
#include "../lb/simple_lb.hpp"
#include "feedback_edge_set_heuristic.hpp"

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
    branches = 0;
    graph.init_crossing_matrix_if_necessary();

    WeightedDirectedGraph weightedDirectedGraph =
        WeightedDirectedGraph::from_matrix(graph.crossing);

    if (weightedDirectedGraph.topologicalSort()) {
        return Order(weightedDirectedGraph.topologicalOrder);
    }

    GeneticHeuristicParameter geneticHeuristicParameter;
    GeneticHeuristic geneticHeuristic([](auto it) { return it <= 2000; },
                                      geneticHeuristicParameter);

    Order goodOrder = geneticHeuristic.solve(graph);
    long crossings = goodOrder.count_crossings(graph);

    SimpleLBParameter parameter;
    parameter.usePotentialMatrix = false;
    auto lb = simpleLB(graph, parameter);

    FeedbackEdgeInstance instance(weightedDirectedGraph, goodOrder,
                                  crossings - lb);

    solveFeedbackEdgeSet(instance);

    auto fesInit = approximateFeedbackEdgeSetFromSolution(
        weightedDirectedGraph, instance.edges, goodOrder);
    addCycleMatrixElements(weightedDirectedGraph, fesInit, instance);

    int iter = 0;
    while (true) {
        iter++;
        solveFeedbackEdgeSet(instance);

        if (instance.ub >= instance.globalUB) {
            std::cerr << "# Branches: " << branches << std::endl;

            return instance.globalUBOrder;
        }

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
            std::cerr << "# Branches: " << branches << std::endl;
            std::cerr << "# Iterations: " << iter << std::endl;
            return Order(G_i.topologicalOrder);
        }

        auto fas = approximateFeedbackEdgeSetFromSolution(G_i, instance.edges,
                                                          goodOrder);
        addCycleMatrixElements(G_i, fas, instance);
    }
}

void FeedbackEdgeSetSolver::solveFeedbackEdgeSet(FeedbackEdgeInstance &instance,
                                                 int k, int cycleSearchStart) {
    branches++;
    int cycleId = -1;
    for (int i = cycleSearchStart; i < instance.circles.size(); i++) {
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

    if (k + lbFeedbackEdgeSet(instance, cycleSearchStart) >= instance.ub) {
        return;
    }

    auto &cycle = instance.circles[cycleId];

    for (auto &edge : cycle->edges) {
        edge->selected = true;
        for (auto &circle : edge->circles) {
            circle->covered++;
        }

        solveFeedbackEdgeSet(instance, k + edge->weight, cycleId + 1);
        edge->selected = false;
        for (auto &circle : edge->circles) {
            circle->covered--;
        }
    }
}

void FeedbackEdgeSetSolver::solveFeedbackEdgeSet(
    FeedbackEdgeInstance &instance) {
    instance.ub = 100000000;
    instance.bestSolution.clear();
    long oldBranches = branches;

    approximateFeedbackEdgeSet(instance);
    std::cerr << "UB: " << instance.ub;
    findGoodCircleOrderForLB(instance);
    std::cerr << " LB: " << lbFeedbackEdgeSet(instance, 0);
    solveFeedbackEdgeSet(instance, 0, 0);
    std::cerr << " Exact: " << instance.ub
              << " Global UB: " << instance.globalUB
              << " Branches: " << branches - oldBranches << std::endl;
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
FeedbackEdgeSetSolver::approximateFeedbackEdgeSetFromSolution(
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

long FeedbackEdgeSetSolver::lbFeedbackEdgeSet(FeedbackEdgeInstance &instance,
                                              int cycleSearchStart) {
    for (auto &e : instance.usedEdges) {
        if (!e->selected) {
            e->potential = e->weight;
        }
    }

    long lb = 0;
    for (int i = cycleSearchStart; i < instance.circles.size(); ++i) {
        auto &c = instance.circles[i];

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
    long bestLB = lbFeedbackEdgeSet(instance, 0);
    std::vector<std::shared_ptr<Circle>> bestCircles = instance.circles;
    for (int i = 0; i < 1000; i++) {

        unsigned seed =
            std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(instance.circles.begin(), instance.circles.end(),
                     std::default_random_engine(seed));

        long lb = lbFeedbackEdgeSet(instance, 0);

        if (lb > bestLB) {
            bestLB = lb;
            bestCircles = instance.circles;
        }
    }

    instance.circles = std::move(bestCircles);

    bool foundImprovement = true;
    while (foundImprovement) {
        foundImprovement = false;

        for (int i = 0; i < instance.circles.size(); ++i) {
            for (int j = i + 1; j < instance.circles.size(); j++) {
                std::swap(instance.circles[i], instance.circles[j]);
                long lb = lbFeedbackEdgeSet(instance, 0);
                if (lb > bestLB) {
                    foundImprovement = true;
                    bestLB = lb;
                } else {
                    std::swap(instance.circles[i], instance.circles[j]);
                }
            }
        }
    }
}

bool FeedbackEdgeInstance::containCircle(Circle &circle) {
    return std::any_of(circles.begin(), circles.end(),
                       [&circle](const std::shared_ptr<Circle> &c) {
                           return c->edges == circle.edges;
                       });
}
