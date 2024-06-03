
#include "feedback_edge_set_heuristic.hpp"

long calculateCost(std::vector<std::shared_ptr<Edge>> &sol) {
    long cost = 0;
    for (auto &e : sol) {
        cost += e->weight;
    }

    return cost;
}

void init_edge_circles(FeedbackEdgeInstance &instance) {
    for (auto &edge : instance.usedEdges) {
        edge->numberOfCircles = 0;
    }
    for (auto &circle : instance.circles) {
        if (circle->covered > 0) {
            continue;
        }

        for (auto &edge : circle->edges) {
            edge->numberOfCircles++;
        }
    }
}

void removeUnnecessaryEdges(FeedbackEdgeInstance &instance,
                            std::vector<std::shared_ptr<Edge>> &solution) {
    std::sort(
        solution.begin(), solution.end(),
        [](const std::shared_ptr<Edge> &a, const std::shared_ptr<Edge> &b) {
            return a->weight > b->weight;
        });

    for (auto it = solution.begin(); it != solution.end();) {
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
            it = solution.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<std::shared_ptr<Edge>>
metaRapsConstruction(FeedbackEdgeInstance &instance,
                     FeedbackEdgeHeuristicParameter &parameter) {
    init_edge_circles(instance);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> functionDistribution(0, 3);
    std::uniform_real_distribution<> probDistribution(0.0, 1.0);

    std::vector<std::shared_ptr<Edge>> solution;

    std::vector<std::shared_ptr<Circle>> uncoveredCircles;

    for (auto &circle : instance.circles) {
        if (circle->covered == 0) {
            uncoveredCircles.push_back(circle);
        }
    }

    std::vector<std::shared_ptr<Edge>> edges;
    for (auto &edge : instance.usedEdges) {
        if (edge->selected) {
            solution.push_back(edge);
        } else {
            edges.push_back(edge);
        }
    }

    while (!uncoveredCircles.empty()) {
        int usedFunction = functionDistribution(gen);

        std::shared_ptr<Edge> minGreedyScore;
        for (auto &edge : edges) {
            if (edge->numberOfCircles == 0) {
                continue;
            }
            auto c = static_cast<double>(edge->weight);
            auto k = static_cast<double>(edge->numberOfCircles);

            switch (usedFunction) {
            case 0:
                edge->greedyValue = c / k;
                break;
            case 1:
                edge->greedyValue = c / (k * k);
                break;
            case 2:
                edge->greedyValue = sqrt(c) / k;
                break;
            case 3:
                edge->greedyValue = c / sqrt(k);
                break;
            default:
                std::cerr << "Unknown function" << std::endl;
                edge->greedyValue = c / k;
            }

            if (minGreedyScore == nullptr ||
                edge->greedyValue < minGreedyScore->greedyValue) {
                minGreedyScore = edge;
            }
        }

        std::shared_ptr<Edge> usedEdge = minGreedyScore;
        if (probDistribution(gen) > parameter.priority) {

            for (auto &edge : edges) {

                int numberOfHits = 0;

                if (edge->greedyValue <=
                    minGreedyScore->greedyValue * (1 + parameter.restriction)) {

                    numberOfHits++;

                    if (probDistribution(gen) < 1.0 / numberOfHits) {
                        usedEdge = edge;
                    }
                }
            }
        }

        usedEdge->selected = true;
        solution.push_back(usedEdge);

        for (auto &circle : usedEdge->circles) {
            circle->covered++;
            if (circle->covered == 1) {
                for (auto &edge : circle->edges) {
                    edge->numberOfCircles--;
                }
            }
        }

        auto _ =
            std::remove_if(uncoveredCircles.begin(), uncoveredCircles.end(),
                           [](std::shared_ptr<Circle> &circle) {
                               return circle->covered > 0;
                           });

        uncoveredCircles.erase(_, uncoveredCircles.end());

        edges.erase(std::remove(edges.begin(), edges.end(), usedEdge),
                    edges.end());
    }

    removeUnnecessaryEdges(instance, solution);

    return solution;
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
neighbourhoodSearch(FeedbackEdgeInstance &instance,
                    std::vector<std::shared_ptr<Edge>> &solution,
                    FeedbackEdgeHeuristicParameter &parameter, long lb) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    std::vector<std::shared_ptr<Edge>> bestSolution = solution;
    long currentCost = calculateCost(bestSolution);

    for (int i = 0; i < parameter.imp_iterations; i++) {

        for (auto &edge : instance.usedEdges) {
            edge->selected = false;
        }

        for (auto &circle : instance.circles) {
            circle->covered = 0;
        }

        for (auto &edge : bestSolution) {
            if (dis(gen) < parameter.search_magnitude) {
                edge->selected = true;
                for (auto &circle : edge->circles) {
                    circle->covered++;
                }
            }
        }

        std::vector<std::shared_ptr<Edge>> newSol =
            metaRapsConstruction(instance, parameter);
        long newCost = calculateCost(newSol);

        if (newCost < currentCost) {
            bestSolution = newSol;
            currentCost = newCost;

            if (currentCost == lb) {
                break;
            }
        }
    }

    return bestSolution;
}

void approximateFeedbackEdgeSet(FeedbackEdgeInstance &instance,
                                FeedbackEdgeHeuristicParameter &parameter,
                                long lb) {
    auto solution = globalApproximateFeedbackEdgeSet(instance);
    long cost = calculateCost(solution);

    if (cost != lb) {
        for (auto &circle : instance.circles) {
            circle->covered = 0;
        }

        for (auto &edge : solution) {
            for (auto &circle : edge->circles) {
                circle->covered++;
            }
        }

        removeUnnecessaryEdges(instance, solution);
        solution = neighbourhoodSearch(instance, solution, parameter, lb);
        for (auto &circle : instance.circles) {
            circle->covered = 0;
        }

        for (auto &circle : instance.usedEdges) {
            circle->selected = false;
        }

        cost = calculateCost(solution);
    }

    instance.ub = cost;
    std::sort(solution.begin(), solution.end());
    instance.bestSolution = solution;
}
