
#include "feedback_edge_set_heuristic.hpp"

long calculateCost(std::vector<std::shared_ptr<Edge>> &sol) {
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

std::vector<std::shared_ptr<Edge>>
greedy2ApproximateFeedbackEdgeSet(FeedbackEdgeInstance &instance) {

    std::vector<int> positionArray(instance.circles.size());
    for (int i = 0; i < instance.circles.size(); ++i) {
        positionArray[i] = i;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(positionArray.begin(), positionArray.end(),
                 std::default_random_engine(seed));

    std::vector<std::shared_ptr<Edge>> solution;

    for (auto &i : positionArray) {
        auto &circle = instance.circles[i];
        if (circle->covered == 0) {
            std::shared_ptr<Edge> usedEdge = circle->edges[0];
            double weightSum = usedEdge->weight;
            for (int j = 1; j < circle->edges.size(); ++j) {
                auto &edge = circle->edges[j];
                weightSum += edge->weight;

                if (dis(gen) < edge->weight / weightSum) {
                    usedEdge = edge;
                }
            }

            for (auto &c : usedEdge->circles) {
                c->covered++;
            }
            solution.emplace_back(usedEdge);
        }
    }

    for (auto &edge : solution) {
        for (auto &c : edge->circles) {
            c->covered--;
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

    for (int _ = 0; _ < 1000; _++) {
        auto greedy2Sol =
            std::move(greedy2ApproximateFeedbackEdgeSet(instance));
        localSearchFeedbackEdgeSet(instance, greedy2Sol);
        long greedy2Cost = calculateCost(greedy2Sol);

        if (greedy2Cost < instance.ub) {
            instance.ub = greedy2Cost;
            instance.bestSolution = std::move(greedy2Sol);
        }
    }
}

bool removeOneEdgeForOneEdge(FeedbackEdgeInstance &instance,
                             std::vector<std::shared_ptr<Edge>> &solution) {

    bool foundImprovement = false;

    for (auto &edgeToRemove : solution) {

        std::shared_ptr<Edge> bestEdgeToAdd = nullptr;
        int count = 0;

        double coverScoreEdgeToRemove = 0;
        for (auto &circle : edgeToRemove->circles) {
            coverScoreEdgeToRemove += 1.0 / circle->edges.size();
        }

        for (auto &edgeToAdd : instance.usedEdges) {
            if (edgeToAdd->selected ||
                edgeToAdd->weight > edgeToRemove->weight) {
                continue;
            }

            int edgeToAddPointer = 0;
            bool covered = true;
            for (auto &circle : edgeToRemove->circles) {
                if (circle->covered == 1) {
                    while (edgeToAddPointer < edgeToAdd->circles.size() &&
                           edgeToAdd->circles[edgeToAddPointer] < circle) {
                        edgeToAddPointer++;
                    }

                    if (edgeToAddPointer == edgeToAdd->circles.size() ||
                        edgeToAdd->circles[edgeToAddPointer] != circle) {
                        covered = false;
                        break;
                    }
                }
            }

            if (covered) {

                if (edgeToRemove->weight == edgeToAdd->weight) {
                    double coverScoreEdgeToAdd = 0;
                    for (auto &circle : edgeToAdd->circles) {
                        coverScoreEdgeToAdd += 1.0 / circle->edges.size();
                    }
                    if (coverScoreEdgeToRemove >= coverScoreEdgeToAdd) {
                        continue;
                    }
                }

                // Edges can be swapped
                count++;
                if (rand() % count == 0) {
                    bestEdgeToAdd = edgeToAdd;
                }
            }
        }

        if (bestEdgeToAdd != nullptr) {
            foundImprovement = true;

            bestEdgeToAdd->selected = true;
            for (auto &circle : bestEdgeToAdd->circles) {
                circle->covered++;
            }

            edgeToRemove->selected = false;
            for (auto &circle : edgeToRemove->circles) {
                circle->covered--;
            }

            edgeToRemove = bestEdgeToAdd;
        }
    }

    return foundImprovement;
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

        foundImprovement =
            removeOneEdgeForOneEdge(instance, solution) || foundImprovement;
    }

    for (auto &edges : solution) {
        edges->selected = false;
        for (auto &circle : edges->circles) {
            circle->covered--;
        }
    }
}
