
#include "feedback_edge_set_solver.hpp"
#include "../heuristic_solver/genetic_algorithm.hpp"
#include "../lb/simple_lb.hpp"
#include "feedback_edge_set_heuristic.hpp"
#include <filesystem>
#include <fstream>
#ifdef USE_ILP_TO_SOLVE_FES
#include "feedback_edge_set_ilp.hpp"
#endif

void Circle::permuteEdges() {
    if (!edges.empty()) {
        std::sort(edges.begin(), edges.end());
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

    auto fesInit = approximateFeedbackEdgeSetFromSolution(
        weightedDirectedGraph, instance.edges, goodOrder);
    addCycleMatrixElements(weightedDirectedGraph, fesInit, instance);

    int iter = 0;
    while (true) {
        iter++;
#ifdef USE_ILP_TO_SOLVE_FES
        solveFeedbackEdgeSetILP(instance);
#else
        solveFeedbackEdgeSet(instance);
#endif

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
    if (k >= instance.ub) {
        return;
    }
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
        for (const auto &edge : instance.usedEdges) {
            if (edge->selected) {
                weight += edge->weight;
                solution.emplace_back(edge);
            }
        }

        if (weight < instance.ub) {
            auto s = solution;
            std::sort(s.begin(), s.end());

            instance.ub = weight;
            instance.bestSolution = solution;
        }
        return;
    }

    if (k + lbFeedbackEdgeSet(instance, cycleSearchStart, false) >=
        instance.ub) {
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
    long oldBranches = branches;

    approximateFeedbackEdgeSet(instance);
    long ub = instance.ub;
    std::cerr << "UB: " << ub;
    findGoodCircleOrderForLB(instance);
    std::cerr << " LB: " << lbFeedbackEdgeSet(instance, 0, false);
    solveFeedbackEdgeSet(instance, 0, 0);
    std::cerr << " Exact: " << instance.ub
              << " Global UB: " << instance.globalUB
              << " Branches: " << branches - oldBranches << std::endl;

    /*if (instance.ub < ub) {
        instance.saveCurrentInstanceToDataset();
    }*/
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

            if (e->weight >= FIXED / 2) {
                continue;
            }

            circle.edges.emplace_back(e);
        }

        circle.permuteEdges();
        if (!instance.containCircle(circle)) {

            auto c = std::make_shared<Circle>(circle);
            instance.circles.emplace_back(c);
            for (auto &e : c->edges) {

                auto it = upper_bound(e->circles.begin(), e->circles.end(), c);
                e->circles.insert(it, c);
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
                                              int cycleSearchStart,
                                              bool useLocalSearch) {
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

            if (min == 0) {
                continue;
            }

            lb += min;
            for (auto &e : c->edges) {
                e->potential -= min;
            }

            c->usedPotential = min;
        }
    }

    if (!useLocalSearch) {
        return lb;
    }

    for (int i = instance.circles.size() - 1; i >= cycleSearchStart; i--) {
        auto cycleToRemove = instance.circles[i];

        if (cycleToRemove->covered > 0 || cycleToRemove->usedPotential == 0) {
            continue;
        }

        long lbChange = 0;

        std::vector<std::tuple<std::shared_ptr<Edge>, int>> edgeChanges;
        std::vector<std::tuple<std::shared_ptr<Circle>, int>> circleChanges;
        std::vector<std::shared_ptr<Edge>> edgesWithNewPotential;

        for (auto &e : cycleToRemove->edges) {

            if (e->selected) {
                continue;
            }

            edgesWithNewPotential.emplace_back(e);
            e->potential += cycleToRemove->usedPotential;
            edgeChanges.emplace_back(e, -cycleToRemove->usedPotential);
        }

        lbChange -= cycleToRemove->usedPotential;

        std::shuffle(
            edgesWithNewPotential.begin(), edgesWithNewPotential.end(),
            std::default_random_engine(
                std::chrono::system_clock::now().time_since_epoch().count()));

        for (auto &e : edgesWithNewPotential) {

            for (auto &c : e->circles) {
                if (c == cycleToRemove || c->covered > 0) {
                    continue;
                }

                if (c->usedPotential ==
                    e->potential - cycleToRemove->usedPotential) {
                    int min = c->edges[0]->potential;
                    for (auto &e : c->edges) {
                        if (e->potential < min) {
                            min = e->potential;
                        }
                    }

                    if (min == 0) {
                        continue;
                    }

                    lbChange += min;
                    for (auto &e : c->edges) {
                        e->potential -= min;
                        edgeChanges.emplace_back(e, min);
                    }

                    c->usedPotential += min;
                    circleChanges.emplace_back(c, -min);
                }
            }
        }

        int min = cycleToRemove->edges[0]->potential;
        for (auto &e : cycleToRemove->edges) {
            if (e->potential < min) {
                min = e->potential;
            }
        }
        lbChange += min;

        if (lbChange <= 0) {
            for (auto &e : edgeChanges) {
                std::get<0>(e)->potential += std::get<1>(e);
            }

            for (auto &c : circleChanges) {
                std::get<0>(c)->usedPotential += std::get<1>(c);
            }
        } else {
            if (min > 0) {
                for (auto &e : cycleToRemove->edges) {
                    e->potential -= min;
                }
            }
            lb += lbChange;

            cycleToRemove->usedPotential = min;
        }
    }

    return lb;
}

void FeedbackEdgeSetSolver::findGoodCircleOrderForLB(
    FeedbackEdgeInstance &instance) {
    long bestLB = lbFeedbackEdgeSet(instance, 0, false);
    std::vector<std::shared_ptr<Circle>> bestCircles = instance.circles;
    for (int i = 0; i < 1000; i++) {

        unsigned seed =
            std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(instance.circles.begin(), instance.circles.end(),
                     std::default_random_engine(seed));

        long lb = lbFeedbackEdgeSet(instance, 0, false);

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
            for (int j = i + 1;
                 j < std::min(i + 20, (int)instance.circles.size()); j++) {
                std::swap(instance.circles[i], instance.circles[j]);
                long lb = lbFeedbackEdgeSet(instance, 0, false);
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

void FeedbackEdgeInstance::writeToFile(std::ostream &gr) {
    int nrEdges = 0;
    for (int i = 0; i < edges.size(); ++i) {
        for (int j = 0; j < edges.size(); ++j) {
            if (edges[i][j] != nullptr && edges[i][j]->weight > 0) {
                nrEdges++;
            }
        }
    }

    gr << "p " << edges.size() << " " << nrEdges << " " << circles.size() << " "
       << globalUB << " " << ub << std::endl;
    for (int i = 0; i < edges.size(); ++i) {
        for (int j = 0; j < edges.size(); ++j) {
            if (edges[i][j]->weight > 0) {
                gr << i << " " << j << " " << edges[i][j]->weight << std::endl;
            }
        }
    }

    for (auto &cycle : circles) {
        for (auto &edge : cycle->edges) {
            gr << edge->start << " " << edge->end << " ";
        }
        gr << std::endl;
    }

    for (const auto &globalOrder : globalUBOrder.position_to_vertex) {
        gr << globalOrder << std::endl;
    }
}
void FeedbackEdgeInstance::saveCurrentInstanceToDataset() {
    int count = 0;
    std::string path = "../data/feedback_edge_set/";

    if (!std::filesystem::exists(path)) {
        if (!std::filesystem::create_directory(path)) {
            std::cerr << "Failed to create the output directory." << std::endl;
        }
    }

    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            count++;
        }
    }

    std::ofstream ofs(path + std::to_string(count) + ".gr");
    writeToFile(ofs);
}
