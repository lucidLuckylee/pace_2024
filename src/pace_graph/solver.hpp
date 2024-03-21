#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "../heuristic_solver/median_position_heuristic.hpp"
#include "order.hpp"
#include "pace_graph.hpp"
#include "solver.hpp"
#include <chrono>

template <typename T> class Solver {
  private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::milliseconds time_limit;

    std::chrono::time_point<std::chrono::steady_clock> start_time_for_part;
    std::chrono::milliseconds time_limit_for_part;
    bool reorderNodes;

  protected:
    virtual void finish(PaceGraph &graph, std::vector<PaceGraph> &subgraphs,
                        std::vector<T> &results,
                        std::vector<int> &isolated_nodes) = 0;
    virtual T run(PaceGraph &graph) = 0;

  public:
    Solver(std::chrono::milliseconds limit = std::chrono::milliseconds::max(),
           bool reorderNodes = false)
        : start_time(std::chrono::steady_clock::now()),
          start_time_for_part(std::chrono::steady_clock::now()),
          time_limit(limit),
          time_limit_for_part(std::chrono::milliseconds::zero()),
          reorderNodes(reorderNodes) {}

    void solve(PaceGraph &graph) {
        auto val = graph.splitGraphOn0Splits();
        auto splittedGraphs = std::get<0>(val);
        auto isolated_nodes = std::get<1>(val);

        if (reorderNodes) {
            CheapHeuristicsParameter cheapHeuristicsParameter;
            MeanPositionSolver meanPositionSolver([this]() { return false; },
                                                  cheapHeuristicsParameter);

            for (int i = 0; i < splittedGraphs.size(); ++i) {
                auto g = splittedGraphs[i];
                auto order = meanPositionSolver.solve(g);
                splittedGraphs[i] = order.reorderGraph(g);
            }
        }

        std::vector<T> results;
        for (int i = 0; i < splittedGraphs.size(); i++) {
            auto g = splittedGraphs[i];

            start_time_for_part = std::chrono::steady_clock::now();

            auto msLeft = time_limit -
                          std::chrono::duration_cast<std::chrono::milliseconds>(
                              start_time_for_part - start_time);

            int sizeForAllUpcomingSegments = 0;
            for (int j = i; j < splittedGraphs.size(); j++) {
                sizeForAllUpcomingSegments += splittedGraphs[j].size_free;
            }

            double percentageForThisSegment =
                static_cast<double>(g.size_free) / sizeForAllUpcomingSegments;

            double newTimeLimitMs = msLeft.count() * percentageForThisSegment;

            time_limit_for_part =
                std::chrono::milliseconds(static_cast<int>(newTimeLimitMs));

            results.push_back(run(g));
        }

        finish(graph, splittedGraphs, results, isolated_nodes);
    }

    bool has_time_left() const {
        return std::chrono::steady_clock::now() - start_time_for_part <
               time_limit_for_part;
    }
};

class SolutionSolver : public Solver<Order> {
  protected:
    void finish(PaceGraph &graph, std::vector<PaceGraph> &subgraphs,
                std::vector<Order> &results,
                std::vector<int> &isolated_nodes) override {
        for (int u : isolated_nodes) {
            std::cout << graph.free_real_names[u] << std::endl;
        }

        for (int i = 0; i < subgraphs.size(); ++i) {
            auto g = subgraphs[i];
            auto sol = results[i];
            std::cout << sol.convert_to_real_node_id(g) << std::endl;
        }
    }

    Order run(PaceGraph &graph) override = 0;

  public:
    explicit SolutionSolver(
        std::chrono::milliseconds limit = std::chrono::milliseconds::max(),
        bool reorderNodes = false)
        : Solver<Order>(limit, reorderNodes) {}
};

#endif // SOLVER_HPP
