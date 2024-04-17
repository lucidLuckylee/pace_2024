#include "feedback_edge_set_heuristic.hpp"
#include "feedback_edge_set_solver.hpp"
int main() {
    std::vector<std::vector<std::shared_ptr<Edge>>> weights;

    std::vector<std::shared_ptr<Circle>> circles;
    int nrNodes = -1;
    int nrEdges = -1;
    int nrCircles = -1;
    long globalUB = -1;

    std::vector<int> globalUBOrder;
    std::string line;
    while (std::getline(std::cin, line)) {

        if (line[0] == 'p') {
            // TODO: Extend for parameterized track with cutwidth
            sscanf(line.c_str(), "p %d %d %d %lu", &nrNodes, &nrEdges,
                   &nrCircles, &globalUB);
            weights.resize(nrNodes);
            for (int i = 0; i < nrNodes; ++i) {
                weights[i].resize(nrNodes);
            }
        } else {

            if (nrEdges > 0) {
                // read in an edge
                int u, v, w;
                sscanf(line.c_str(), "%d %d %d", &u, &v, &w);
                weights[u][v] = std::make_shared<Edge>(u, v, w);
                nrEdges--;
            } else if (nrCircles > 0) {
                std::stringstream ss(line);
                std::vector<int> numbers;

                int num;
                while (ss >> num) {
                    numbers.push_back(num);
                }

                auto circle = std::make_shared<Circle>();
                for (int i = 0; i < numbers.size(); i += 2) {
                    int u = numbers[i];
                    int v = numbers[i + 1];
                    if (weights[u][v]->weight >= FIXED / 2)
                        continue;
                    circle->edges.emplace_back(weights[u][v]);
                }
                circle->permuteEdges();
                circles.push_back(circle);

                for (auto &e : circle->edges) {
                    auto it = upper_bound(e->circles.begin(), e->circles.end(),
                                          circle);
                    e->circles.insert(it, circle);
                    e->numberOfCircles++;
                }
                nrCircles--;
            } else {
                int u;
                sscanf(line.c_str(), "%d", &u);
                globalUBOrder.push_back(u);
            }
        }
    }

    Order order(globalUBOrder);
    FeedbackEdgeInstance instance(weights, circles, order, globalUB);

    for (auto &circle : instance.circles) {
        for (auto &edge : circle->edges) {
            instance.usedEdges.insert(edge);
        }
    }

    /*FeedbackEdgeSetSolver solver;
    solver.solveFeedbackEdgeSet(instance);*/
    approximateFeedbackEdgeSet(instance);

    std::cout << "#Size: " << instance.ub << std::endl;

    for (auto &edge : instance.bestSolution) {
        std::cout << edge->start << " " << edge->end << std::endl;
    }

    return 0;
}