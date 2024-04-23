#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "../data_reduction/data_reduction_rules.hpp"
#include "../heuristic_solver/mean_position_heuristic.hpp"
#include "directed_graph.hpp"
#include "order.hpp"
#include "pace_graph.hpp"
#include <chrono>

enum ReorderType { REORDER_NONE, REORDER_HEURISTIC, REORDER_FIXED_NODE_SET };

template <typename T> class Solver {
  private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::milliseconds time_limit;

    std::chrono::time_point<std::chrono::steady_clock> start_time_for_part;
    std::chrono::milliseconds time_limit_for_part;
    ReorderType reorderNodes;
    bool initUB;

  protected:
    double percentage_for_this_part;

    virtual void finish(PaceGraph &graph,
                        std::vector<std::unique_ptr<PaceGraph>> &subgraphs,
                        std::vector<T> &results,
                        std::vector<int> &isolated_nodes) = 0;
    virtual T run(PaceGraph &graph) = 0;

  public:
    Solver(std::chrono::milliseconds limit = std::chrono::milliseconds::max(),
           ReorderType reorderNodes = REORDER_NONE, bool initUB = true)
        : start_time(std::chrono::steady_clock::now()),
          start_time_for_part(std::chrono::steady_clock::now()),
          time_limit(limit),
          time_limit_for_part(std::chrono::milliseconds::zero()),
          reorderNodes(reorderNodes), initUB(initUB) {}

    void solve(PaceGraph &graph) {
        std::tuple<std::vector<std::unique_ptr<PaceGraph>>, std::vector<int>>
            val = graph.splitGraphs();
        std::vector<std::unique_ptr<PaceGraph>> splittedGraphs =
            std::move(std::get<0>(val));

        auto isolated_nodes = std::move(std::get<1>(val));

        std::vector<T> results;

        // solve all nodes with <= 2 free nodes directly
        for (const auto &g : splittedGraphs) {
            if (g->size_free <= 2) {
                if (g->size_free == 1) {
                    g->remove_free_vertices({{0, 0, 0}});
                } else if (g->size_free == 2) {
                    auto [cost_1_2, cost_2_1] =
                        g->calculatingCrossingNumber(0, 1);

                    if (cost_1_2 < cost_2_1) {
                        g->remove_free_vertices({{0, 0, cost_1_2}});
                        g->remove_free_vertices({{1, 1, 0}});
                    } else {
                        g->remove_free_vertices({{1, 0, cost_2_1}});
                        g->remove_free_vertices({{0, 1, 0}});
                    }
                }
            }
        }

        for (int i = 0; i < splittedGraphs.size(); i++) {
            auto &g = splittedGraphs[i];
            if (g->size_free == 0) {
                results.push_back(run(*g));
                continue;
            }
            if (reorderNodes == REORDER_HEURISTIC || initUB) {

                MeanPositionParameter meanPositionParameter;
                auto start_time = std::chrono::steady_clock::now();
                MeanPositionSolver meanPositionSolver(
                    [this, start_time](int it) {
                        auto current_time = std::chrono::steady_clock::now();
                        auto diff = current_time - start_time;
                        if (diff > std::chrono::milliseconds(5000)) {
                            return false;
                        }
                        return it == 0;
                    },
                    meanPositionParameter);

                auto order = meanPositionSolver.solve(*g);
                long ub = order.count_crossings(*g);

                if (reorderNodes == REORDER_HEURISTIC) {
                    g = order.reorderGraph(*g);
                }

                if (initUB) {
                    g->ub = ub;
                }
            }

            if (reorderNodes == REORDER_FIXED_NODE_SET) {
                std::vector<int> newNodeOrder;

                std::vector<bool> alreadyUsed(g->size_free, false);

                for (auto &neig : g->neighbors_fixed) {
                    for (auto &v : neig) {
                        if (!alreadyUsed[v]) {
                            newNodeOrder.push_back(v);
                            alreadyUsed[v] = true;
                        }
                    }
                }

                g = Order(newNodeOrder).reorderGraph(*g);
            }

            start_time_for_part = std::chrono::steady_clock::now();
            apply_reduction_rules(*g);

            auto msLeft = time_limit -
                          std::chrono::duration_cast<std::chrono::milliseconds>(
                              start_time_for_part - start_time);

            int sizeForAllUpcomingSegments = 0;
            for (int j = i; j < splittedGraphs.size(); j++) {
                sizeForAllUpcomingSegments += splittedGraphs[j]->size_free;
            }

            percentage_for_this_part =
                static_cast<double>(g->size_free) / sizeForAllUpcomingSegments;

            double newTimeLimitMs = msLeft.count() * percentage_for_this_part;

            time_limit_for_part =
                std::chrono::milliseconds(static_cast<int>(newTimeLimitMs));

            results.push_back(run(*g));
        }

        finish(graph, splittedGraphs, results, isolated_nodes);
        auto current_time = std::chrono::steady_clock::now();
        std::cerr << "# Time: "
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
    void finish(PaceGraph &graph,
                std::vector<std::unique_ptr<PaceGraph>> &subgraphs,
                std::vector<Order> &results,
                std::vector<int> &isolated_nodes) override {

        for (const auto &u : isolated_nodes) {
            std::cout << graph.free_real_names[u] << std::endl;
        }

        long crossings = graph.cost_through_deleted_nodes;

        for (int i = 0; i < subgraphs.size(); ++i) {
            auto &g = subgraphs[i];
            auto &sol = results[i];

            // crossings += sol.count_crossings(*g);
            crossings += g->cost_through_deleted_nodes;
            std::vector<int> sub_solution = sol.position_to_vertex;

            for (int j = 0; j < g->size_free; ++j) {
                sub_solution[j] = g->free_real_names[sub_solution[j]];
            }

            auto removed_vertices = g->removed_vertices;
            while (!removed_vertices.empty()) {
                auto [v, position] = removed_vertices.top();
                removed_vertices.pop();
                sub_solution.insert(sub_solution.begin() + position, v);
            }

            for (const auto &u : sub_solution) {
                std::cout << u << std::endl;
            }
        }

        std::cerr << "#Crossings: " << crossings << std::endl;
    }

    Order run(PaceGraph &graph) override = 0;

  public:
    explicit SolutionSolver(
        std::chrono::milliseconds limit = std::chrono::milliseconds::max(),
        ReorderType reorderNodes = REORDER_NONE)
        : Solver<Order>(limit, reorderNodes) {}
};

#endif // SOLVER_HPP
