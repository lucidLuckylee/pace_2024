
#include "feedback_edge_set_solver.hpp"
#include "../heuristic_solver/genetic_algorithm.hpp"
#include "../heuristic_solver/greedy_insert_solver.hpp"
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

    std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();

    std::chrono::milliseconds time_for_heuristic =
        std::chrono::milliseconds(static_cast<int>(
            time_limit_left_for_heuristic.count() * percentage_for_this_part));
    time_limit_left_for_heuristic -= time_for_heuristic;
    Order goodOrder = Order(0);

    GeneticHeuristicParameter geneticHeuristicParameter;
    GeneticHeuristic geneticHeuristic(
        [start, time_for_heuristic, this](auto it) {
            if (this->fes_parameter.useFastHeuristic) {
                return it == 0;
            }
            auto now = std::chrono::steady_clock::now();
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                                      start);
            return elapsed < time_for_heuristic;
        },
        geneticHeuristicParameter);

    goodOrder = geneticHeuristic.solve(graph);

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
        if (instance.globalUB < 10) {
            solveFeedbackEdgeSet(instance);
        } else {
            solveFeedbackEdgeSetILP(instance);
        }
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
    long oldBranches = branches;
    long lb = lbFeedbackEdgeSet(instance, 0);

    FeedbackEdgeHeuristicParameter heuristicParameter;
    approximateFeedbackEdgeSet(instance, heuristicParameter, lb);
    long ub = instance.ub;
    std::cerr << "UB: " << ub;

    if (lb == ub) {
        return;
    }

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
        for (int j = 0; j < i; j++) {
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
    return lb;
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
