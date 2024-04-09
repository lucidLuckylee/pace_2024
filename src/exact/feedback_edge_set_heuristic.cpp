
#include "feedback_edge_set_heuristic.hpp"

void globalApproximateFeedbackEdgeSet(FeedbackEdgeInstance &instance) {

    long weight = 0;
    std::vector<std::shared_ptr<Edge>> feedbackEdgeSet;
    for (auto &e : instance.usedEdges) {
        bool usedInGlobalUB = instance.globalUBOrder.get_position(e->end) <
                              instance.globalUBOrder.get_position(e->start);

        if (usedInGlobalUB) {
            weight += e->weight;
            feedbackEdgeSet.emplace_back(e);
        }
    }

    if (weight < instance.ub) {
        instance.ub = weight;
        instance.bestSolution = feedbackEdgeSet;
    }
}

void greedyApproximateFeedbackEdgeSet(FeedbackEdgeInstance &instance) {
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

        if (weight < instance.ub) {
            instance.ub = weight;
            instance.bestSolution = solution;
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

    greedyApproximateFeedbackEdgeSet(instance);

    edge->selected = false;
    for (auto &c : edge->circles) {
        c->covered--;
        for (auto &e : c->edges) {
            e->numberOfCircles++;
        }
    }
}
void approximateFeedbackEdgeSet(FeedbackEdgeInstance &instance) {
    greedyApproximateFeedbackEdgeSet(instance);
    globalApproximateFeedbackEdgeSet(instance);
    localSearchFeedbackEdgeSet(instance);
}
void localSearchFeedbackEdgeSet(FeedbackEdgeInstance &instance) {

    for (auto &edges : instance.bestSolution) {
        edges->selected = true;
        for (auto &circle : edges->circles) {
            circle->covered++;
        }
    }

    // Test if I can remove an edge
    for (auto it = instance.bestSolution.begin();
         it != instance.bestSolution.end();) {
        bool canRemove = true;
        for (auto &circle : (*it)->circles) {
            if (circle->covered == 1) {
                canRemove = false;
                break;
            }
        }

        if (canRemove) {
            (*it)->selected = false;
            for (auto &circle : (*it)->circles) {
                circle->covered--;
            }
            instance.ub -= (*it)->weight;
            // remove the edge from bestSolution
            it = instance.bestSolution.erase(it);
        } else {
            ++it;
        }
    }

    // Test if I can remove one edge for one edge with less weight
    for (auto &edge : instance.bestSolution) {
        std::vector<std::shared_ptr<Edge>> edgesThatCanBeUsedToReplace;

        for (auto &circle : edge->circles) {
            if (circle->covered == 1) {
                if (edgesThatCanBeUsedToReplace.empty()) {
                    edgesThatCanBeUsedToReplace = circle->edges;
                    std::sort(edgesThatCanBeUsedToReplace.begin(),
                              edgesThatCanBeUsedToReplace.end());
                } else {
                    std::vector<std::shared_ptr<Edge>>
                        newEdgesThatCanBeUsedToReplace;

                    auto newEdges = circle->edges;
                    std::sort(newEdges.begin(), newEdges.end());

                    std::set_intersection(
                        edgesThatCanBeUsedToReplace.begin(),
                        edgesThatCanBeUsedToReplace.end(), newEdges.begin(),
                        newEdges.end(),
                        std::back_inserter(newEdgesThatCanBeUsedToReplace));
                    edgesThatCanBeUsedToReplace =
                        std::move(newEdgesThatCanBeUsedToReplace);
                }
            }
        }

        for (auto &e : edgesThatCanBeUsedToReplace) {
            if (e->weight < edge->weight && !e->selected) {
                e->selected = true;

                for (auto &circle : e->circles) {
                    circle->covered++;
                }

                edge->selected = false;
                for (auto &circle : edge->circles) {
                    circle->covered--;
                }

                edge = e;
                instance.ub -= edge->weight;
            }
        }
    }

    for (auto &edges : instance.bestSolution) {
        edges->selected = false;
        for (auto &circle : edges->circles) {
            circle->covered--;
        }
    }
}
