
#ifndef PACE2024_FEEDBACK_EDGE_SET_SOLVER_HPP
#define PACE2024_FEEDBACK_EDGE_SET_SOLVER_HPP

#include "../pace_graph/solver.hpp"

class FESParameter {
  public:
    bool useFastHeuristic = true;
};

class Edge;

class Circle {
  public:
    int covered = 0;
    int usedPotential = 0;
    std::vector<std::shared_ptr<Edge>> edges;
    void *ilpConstraint = nullptr;

    Circle() = default;

    void permuteEdges();
};

class Edge {
  public:
    int start, end, weight;
    std::vector<std::shared_ptr<Circle>> circles;
    bool selected = false;
    int numberOfCircles = 0;
    int potential = 0;
    double greedyValue = 0;
    void *ilpVar = nullptr;

    Edge(int start, int end, int weight)
        : start(start), end(end), weight(weight) {}

    bool operator==(const Edge &other) const {
        return start == other.start && end == other.end;
    }
};

class FeedbackEdgeInstance {
  public:
    std::vector<std::shared_ptr<Circle>> circles;

    std::vector<std::vector<std::shared_ptr<Edge>>> edges;

    std::unordered_set<std::shared_ptr<Edge>> usedEdges;
    std::vector<std::shared_ptr<Edge>> bestSolution;

    long ub = 0;

    long globalUB = 0;
    Order globalUBOrder;
    void *ilpModel = nullptr;

    explicit
    FeedbackEdgeInstance(std::vector<std::vector<std::shared_ptr<Edge>>> &edges,
                         std::vector<std::shared_ptr<Circle>> &circles,
                         Order &globalUBOrder, long globalUB)
        : edges(edges), circles(circles), globalUB(globalUB),
          globalUBOrder(globalUBOrder){};

    explicit FeedbackEdgeInstance(WeightedDirectedGraph &graph, Order &order,
                                  long globalUB)
        : globalUB(globalUB), globalUBOrder(order) {
        edges.resize(graph.neighbors.size());
        for (int i = 0; i < graph.neighbors.size(); ++i) {
            for (int j = 0; j < graph.neighbors.size(); ++j) {
                edges[i].emplace_back(
                    std::make_shared<Edge>(i, j, graph.weights[i][j]));
            }
        }
    }

    bool containCircle(Circle &circle);
    void writeToFile(std::ostream &gr);

    void saveCurrentInstanceToDataset();
};

class FeedbackEdgeSetSolver : public SolutionSolver {
  private:
    long branches;
    std::chrono::milliseconds time_limit_left_for_heuristic =
        std::chrono::milliseconds(60 * 1000);

    void
    addCycleMatrixElements(DirectedGraph &graph,
                           std::vector<std::shared_ptr<Edge>> &feedbackEdgeSet,
                           FeedbackEdgeInstance &instance);

    void solveFeedbackEdgeSet(FeedbackEdgeInstance &instance, int k,
                              int cycleSearchStart);

    Order tryToSolveByMatchingUBAndLB(PaceGraph &graph);

    std::vector<std::shared_ptr<Edge>> approximateFeedbackEdgeSetFromSolution(
        DirectedGraph &graph,
        std::vector<std::vector<std::shared_ptr<Edge>>> &edges, Order &order);

    FESParameter fes_parameter;

  protected:
    Order run(PaceGraph &graph) override;

  public:
    void solveFeedbackEdgeSet(FeedbackEdgeInstance &instance);

    long lbFeedbackEdgeSet(FeedbackEdgeInstance &instance,
                           int cycleSearchStart);

    explicit FeedbackEdgeSetSolver(
        FESParameter fes_parameter,
        std::chrono::milliseconds limit = std::chrono::milliseconds::max())
        : fes_parameter(fes_parameter), SolutionSolver(limit) {}
};

#endif // PACE2024_FEEDBACK_EDGE_SET_SOLVER_HPP
