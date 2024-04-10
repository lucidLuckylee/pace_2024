
#include "feedback_edge_set_heuristic.hpp"

long calculateCost(std::vector<std::shared_ptr<Edge>> sol) {
    long cost = 0;
    for (auto &e : sol) {
        cost += e->weight;
    }

    return cost;
}

std::vector<std::shared_ptr<Edge>>
globalApproximateFeedbackEdgeSet(FeedbackEdgeInstance &instance) {

    std::vector<std::shared_ptr<Edge>> feedbackEdgeSet;
    for (auto &e : instance.usedEdges) {
        bool usedInGlobalUB = instance.globalUBOrder.get_position(e->end) <
                              instance.globalUBOrder.get_position(e->start);

        if (usedInGlobalUB) {
            feedbackEdgeSet.emplace_back(e);
        }
    }

    return feedbackEdgeSet;
}

std::vector<std::shared_ptr<Edge>>
greedyApproximateFeedbackEdgeSet(FeedbackEdgeInstance &instance) {
    std::shared_ptr<Edge> edge = nullptr;
    double bestCost = 100000000;
    for (auto &e : instance.usedEdges) {

        if (e->numberOfCircles > 0 && !e->selected) {
            double cost = static_cast<double>(e->weight) / (e->numberOfCircles);
            if (cost < bestCost) {
                bestCost = cost;
                edge = e;
            }
        }
    }

    if (edge == nullptr) {
        std::vector<std::shared_ptr<Edge>> solution;
        long weight = 0;
        for (auto &edge : instance.usedEdges) {

            if (edge->selected) {
                weight += edge->weight;
                solution.emplace_back(edge);
            }
        }

        return solution;
    }

    edge->selected = true;
    for (auto &c : edge->circles) {
        c->covered++;
        for (auto &e : c->edges) {
            e->numberOfCircles--;
        }
    }

    auto solution = std::move(greedyApproximateFeedbackEdgeSet(instance));

    edge->selected = false;
    for (auto &c : edge->circles) {
        c->covered--;
        for (auto &e : c->edges) {
            e->numberOfCircles++;
        }
    }

    return solution;
}
void approximateFeedbackEdgeSet(FeedbackEdgeInstance &instance) {
    auto lastSolution = instance.bestSolution;

    auto greedySol = std::move(greedyApproximateFeedbackEdgeSet(instance));
    localSearchFeedbackEdgeSet(instance, greedySol);
    long greedyCost = calculateCost(greedySol);

    auto globalSol = globalApproximateFeedbackEdgeSet(instance);
    localSearchFeedbackEdgeSet(instance, globalSol);
    long globalCost = calculateCost(globalSol);

    if (greedyCost < globalCost) {
        instance.ub = greedyCost;
        instance.bestSolution = std::move(greedySol);
    } else {
        instance.ub = globalCost;
        instance.bestSolution = std::move(globalSol);
    }
}
void localSearchFeedbackEdgeSet(FeedbackEdgeInstance &instance,
                                std::vector<std::shared_ptr<Edge>> &solution) {

    for (auto &edges : solution) {
        edges->selected = true;
        for (auto &circle : edges->circles) {
            circle->covered++;
        }
    }

    bool foundImprovement = true;
    while (foundImprovement) {
        foundImprovement = false;

        std::shuffle(solution.begin(), solution.end(),
                     std::default_random_engine(time(nullptr)));

        // Test if I can remove an edge
        for (auto it = solution.begin(); it != solution.end();) {
            bool canRemove = true;
            for (auto &circle : (*it)->circles) {
                if (circle->covered == 1) {
                    canRemove = false;
                    break;
                }
            }

            if (canRemove) {
                foundImprovement = true;
                (*it)->selected = false;
                for (auto &circle : (*it)->circles) {
                    circle->covered--;
                }
                // remove the edge from bestSolution
                it = solution.erase(it);
            } else {
                ++it;
            }
        }

        // Test if I can remove one edge for one edge with less weight
        for (auto &edge : solution) {
            std::vector<std::shared_ptr<Edge>> edgesThatCanBeUsedToReplace;

            for (auto &circle : edge->circles) {
                if (circle->covered == 1) {
                    if (edgesThatCanBeUsedToReplace.empty()) {
                        edgesThatCanBeUsedToReplace = circle->edges;

                    } else {
                        std::vector<std::shared_ptr<Edge>>
                            newEdgesThatCanBeUsedToReplace;

                        std::set_intersection(
                            edgesThatCanBeUsedToReplace.begin(),
                            edgesThatCanBeUsedToReplace.end(),
                            circle->edges.begin(), circle->edges.end(),
                            std::back_inserter(newEdgesThatCanBeUsedToReplace));
                        edgesThatCanBeUsedToReplace =
                            std::move(newEdgesThatCanBeUsedToReplace);
                    }
                }
            }

            for (auto &e : edgesThatCanBeUsedToReplace) {
                if (e->weight < edge->weight && !e->selected) {
                    e->selected = true;
                    foundImprovement = true;

                    for (auto &circle : e->circles) {
                        circle->covered++;
                    }

                    edge->selected = false;
                    for (auto &circle : edge->circles) {
                        circle->covered--;
                    }

                    edge = e;
                }
            }
        }

        // Test if I can remove two edges for one
        for (int i = 0; i < solution.size(); i++) {
            for (int j = i + 1; j < solution.size(); j++) {
                auto &e1 = solution[i];
                auto &e2 = solution[j];

                std::vector<std::shared_ptr<Circle>>
                    circlesNotCoveredWhenRemoveE1AndE2;

                int pointerE1 = 0;
                int pointerE2 = 0;
                while (true) {
                    if (pointerE1 == e1->circles.size() ||
                        pointerE2 == e2->circles.size()) {
                        break;
                    }

                    if (e1->circles[pointerE1] == e2->circles[pointerE2]) {
                        if (e1->circles[pointerE1]->covered == 2) {
                            circlesNotCoveredWhenRemoveE1AndE2.emplace_back(
                                e1->circles[pointerE1]);
                        }
                        pointerE1++;
                        pointerE2++;
                    } else if (e1->circles[pointerE1]->covered <
                               e2->circles[pointerE2]->covered) {
                        if (e1->circles[pointerE1]->covered == 1) {
                            circlesNotCoveredWhenRemoveE1AndE2.emplace_back(
                                e1->circles[pointerE1]);
                        }
                        pointerE1++;
                    } else {
                        if (e2->circles[pointerE2]->covered == 1) {
                            circlesNotCoveredWhenRemoveE1AndE2.emplace_back(
                                e2->circles[pointerE2]);
                        }
                        pointerE2++;
                    }
                }

                for (int u = pointerE1; u < e1->circles.size(); u++) {
                    if (e1->circles[u]->covered == 1) {
                        circlesNotCoveredWhenRemoveE1AndE2.emplace_back(
                            e1->circles[u]);
                    }
                }

                for (int u = pointerE2; u < e2->circles.size(); u++) {
                    if (e2->circles[u]->covered == 1) {
                        circlesNotCoveredWhenRemoveE1AndE2.emplace_back(
                            e2->circles[u]);
                    }
                }

                for (auto &e : instance.usedEdges) {
                    if (!e->selected && e->weight < e1->weight + e2->weight &&
                        std::includes(
                            e->circles.begin(), e->circles.end(),
                            circlesNotCoveredWhenRemoveE1AndE2.begin(),
                            circlesNotCoveredWhenRemoveE1AndE2.end())) {
                        e->selected = true;
                        e1->selected = false;
                        e2->selected = false;
                        foundImprovement = true;

                        for (auto &circle : e->circles) {
                            circle->covered++;
                        }

                        for (auto &circle : e1->circles) {
                            circle->covered--;
                        }

                        for (auto &circle : e2->circles) {
                            circle->covered--;
                        }

                        solution.erase(solution.begin() + j);
                        solution.erase(solution.begin() + i);
                        solution.insert(solution.begin() + i, e);

                        break;
                    }
                }
            }
        }
    }

    for (auto &edges : solution) {
        edges->selected = false;
        for (auto &circle : edges->circles) {
            circle->covered--;
        }
    }
}
