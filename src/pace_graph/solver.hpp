#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "../data_reduction/data_reduction_rules.hpp"
#include "../heuristic_solver/mean_position_heuristic.hpp"
#include "directed_graph.hpp"
#include "order.hpp"
#include "pace_graph.hpp"
#include <chrono>

template <typename T> class Solver {
  private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::milliseconds time_limit;

    std::chrono::time_point<std::chrono::steady_clock> start_time_for_part;
    std::chrono::milliseconds time_limit_for_part;
    bool reorderNodes;
    bool initUB;

  protected:
    virtual void finish(PaceGraph &graph, std::vector<PaceGraph> &subgraphs,
                        std::vector<T> &results,
                        std::vector<int> &isolated_nodes) = 0;
    virtual T run(PaceGraph &graph) = 0;

  public:
    Solver(std::chrono::milliseconds limit = std::chrono::milliseconds::max(),
           bool reorderNodes = false, bool initUB = true)
        : start_time(std::chrono::steady_clock::now()),
          start_time_for_part(std::chrono::steady_clock::now()),
          time_limit(limit),
          time_limit_for_part(std::chrono::milliseconds::zero()),
          reorderNodes(reorderNodes), initUB(initUB) {}

    void solve(PaceGraph &graph) {
        auto val = graph.splitGraphs();
        auto splittedGraphs = std::get<0>(val);
        auto isolated_nodes = std::get<1>(val);

        if (reorderNodes || initUB) {
            MeanPositionParameter meanPositionParameter;
            MeanPositionSolver meanPositionSolver(
                [this](int it) { return it == 0; }, meanPositionParameter);

            for (int i = 0; i < splittedGraphs.size(); ++i) {
                auto g = splittedGraphs[i];
                auto order = meanPositionSolver.solve(g);
                long ub = order.count_crossings(g);

                if (reorderNodes) {
                    splittedGraphs[i] = order.reorderGraph(g);
                }

                if (initUB) {
                    splittedGraphs[i].ub = ub;
                }
            }
        }

        std::vector<T> results;
        for (int i = 0; i < splittedGraphs.size(); i++) {
            auto &g = splittedGraphs[i];

            start_time_for_part = std::chrono::steady_clock::now();
            apply_reduction_rules(g);

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
        auto current_time = std::chrono::steady_clock::now();
        std::cout << "# Time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         current_time - start_time)
                         .count()
                  << "ms" << std::endl;
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

        for (const auto &u : isolated_nodes) {
            std::cout << graph.free_real_names[u] << std::endl;
        }

        long crossings = graph.cost_through_deleted_nodes;

        for (int i = 0; i < subgraphs.size(); ++i) {
            auto &g = subgraphs[i];
            auto &sol = results[i];

            crossings += sol.count_crossings(g);
            crossings += g.cost_through_deleted_nodes;
            std::vector<int> sub_solution = sol.position_to_vertex;

            for (int j = 0; j < g.size_free; ++j) {
                sub_solution[j] = g.free_real_names[sub_solution[j]];
            }

            auto removed_vertices = g.removed_vertices;
            while (!removed_vertices.empty()) {
                auto [v, position] = removed_vertices.top();
                removed_vertices.pop();
                sub_solution.insert(sub_solution.begin() + position, v);
            }

            for (const auto &u : sub_solution) {
                std::cout << u << std::endl;
            }
        }

        std::cout << "#Crossings: " << crossings << std::endl;
    }

    Order run(PaceGraph &graph) override = 0;

  public:
    explicit SolutionSolver(
        std::chrono::milliseconds limit = std::chrono::milliseconds::max(),
        bool reorderNodes = false)
        : Solver<Order>(limit, reorderNodes) {}
};

#endif // SOLVER_HPP
