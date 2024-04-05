
#ifndef PACE2024_FEEDBACK_EDGE_SET_SOLVER_HPP
#define PACE2024_FEEDBACK_EDGE_SET_SOLVER_HPP

#include "../pace_graph/solver.hpp"

class Edge;

class Circle {
  public:
    int covered = 0;
    std::vector<std::shared_ptr<Edge>> edges;

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

    explicit FeedbackEdgeInstance(WeightedDirectedGraph &graph) {
        edges.resize(graph.neighbors.size());
        for (int i = 0; i < graph.neighbors.size(); ++i) {
            for (int j = 0; j < graph.neighbors.size(); ++j) {
                edges[i].emplace_back(
                    std::make_shared<Edge>(i, j, graph.weights[i][j]));
            }
        }
    }

    bool containCircle(Circle &circle);
};

class FeedbackEdgeSetSolver : public SolutionSolver {
  private:
    void
    addCycleMatrixElements(DirectedGraph &graph,
                           std::vector<std::shared_ptr<Edge>> &feedbackEdgeSet,
                           FeedbackEdgeInstance &instance);

    void solveFeedbackEdgeSet(FeedbackEdgeInstance &instance);

    void solveFeedbackEdgeSet(FeedbackEdgeInstance &instance, int k);

    std::vector<std::shared_ptr<Edge>> approximateFeedbackEdgeSet(
        DirectedGraph &graph,
        std::vector<std::vector<std::shared_ptr<Edge>>> &edges);

    void approximateFeedbackEdgeSet(FeedbackEdgeInstance &instance);

    long lbFeedbackEdgeSet(FeedbackEdgeInstance &instance);

  protected:
    Order run(PaceGraph &graph) override;

  public:
    explicit FeedbackEdgeSetSolver(
        std::chrono::milliseconds limit = std::chrono::milliseconds::max())
        : SolutionSolver(limit) {}
};

#endif // PACE2024_FEEDBACK_EDGE_SET_SOLVER_HPP
