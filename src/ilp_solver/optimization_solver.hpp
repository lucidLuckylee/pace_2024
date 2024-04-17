//
// Created by jt on 27.03.24.
//

#ifndef PACE2024_OPTIMIZATION_SOLVER_HPP
#define PACE2024_OPTIMIZATION_SOLVER_HPP

#include "../pace_graph/order.hpp"
#include "../pace_graph/pace_graph.hpp"
#include "../pace_graph/solver.hpp"

template <typename MODEL, typename VAR, typename OPT_FUN, typename MODEL_RESULT>
class OptimizationSolver : public SolutionSolver {
  protected:
    virtual MODEL createModel() = 0;
    virtual OPT_FUN createOptFunctions(MODEL &model) = 0;
    virtual void addOptFunctions(MODEL &model, OPT_FUN optFun) = 0;

    virtual VAR createVariable(MODEL &model, std::string name) = 0;
    virtual void addSymmetryConstraints(MODEL &model, VAR &var1, VAR &var2) = 0;
    virtual void addTransitiveConstraints(MODEL &model, VAR &var1, VAR &var2,
                                          VAR &var3) = 0;

    virtual void setVariable(MODEL &model, VAR &var, bool val) = 0;
    virtual void setCoefficient(MODEL &model, OPT_FUN &optFun, VAR &var,
                                int crossingMatrix) = 0;

    virtual MODEL_RESULT solveModel(MODEL &model) = 0;
    virtual bool getVariableValue(MODEL &mode, MODEL_RESULT &result,
                                  VAR &var) = 0;

    virtual void setThreadLimit(MODEL &model, int limit) = 0;

  public:
    explicit OptimizationSolver(
        std::chrono::milliseconds limit = std::chrono::milliseconds::max())
        : SolutionSolver(limit, REORDER_FIXED_NODE_SET) {}

    Order run(PaceGraph &graph) override;
};

template <typename MODEL, typename VAR, typename OPT_FUN, typename MODEL_RESULT>
Order OptimizationSolver<MODEL, VAR, OPT_FUN, MODEL_RESULT>::run(
    PaceGraph &graph) {
    std::vector<std::vector<VAR>> variable_matrix(graph.size_free);
    MODEL model = createModel();
    setThreadLimit(model, 1);

    OPT_FUN optFun = createOptFunctions(model);
    for (int i = 0; i < graph.size_free; ++i) {
        variable_matrix[i].resize(graph.size_free);
        for (int j = 0; j < graph.size_free; ++j) {
            variable_matrix[i][j] = createVariable(
                model, "m_" + std::to_string(i) + "_" + std::to_string(j));

            if (graph.crossing.comparable(i, j)) {

                bool isOne = graph.crossing.lt(i, j);

                setVariable(model, variable_matrix[i][j], isOne);
            } else {
                setCoefficient(model, optFun, variable_matrix[i][j],
                               graph.crossing.matrix[i][j]);
            }
        }
    }
    addOptFunctions(model, optFun);

    for (int i = 0; i < graph.size_free; ++i) {
        for (int j = i + 1; j < graph.size_free; ++j) {
            if (graph.crossing.comparable(i, j))
                continue;

            auto varij = variable_matrix[i][j];
            auto varji = variable_matrix[j][i];

            addSymmetryConstraints(model, varij, varji);
        }
    }

    bool is_transitivity;
    int transitivity_constraints = 0;
    MODEL_RESULT res;
    do {
        res = solveModel(model);
        is_transitivity = true;

        std::vector<std::bitset<20000>> edges(graph.size_free);

        for (int i = 0; i < graph.size_free; ++i) {
            for (int j = 0; j < graph.size_free; ++j) {
                if (i == j)
                    continue;

                if (getVariableValue(model, res, variable_matrix[i][j]) > 0.5) {
                    edges[i].set(j);
                }
            }
        }

        for (int i = 0; i < graph.size_free; ++i) {
            auto &edgesI = edges[i];

            for (int j = edgesI._Find_first(); j < graph.size_free;
                 j = edgesI._Find_next(j)) {
                auto &edgesJ = edges[j];

                auto ks = ~edgesI & edgesJ;

                for (int k = ks._Find_first(); k < graph.size_free;
                     k = ks._Find_next(k)) {
                    if (k == i || k == j)
                        continue;

                    auto varij = variable_matrix[i][j];
                    auto varjk = variable_matrix[j][k];
                    auto varik = variable_matrix[i][k];

                    addTransitiveConstraints(model, varij, varjk, varik);
                    transitivity_constraints++;
                    is_transitivity = false;
                }
            }
        }
    } while (!is_transitivity);

    std::vector<int> solution(graph.size_free);

    for (int i = 0; i < graph.size_free; ++i) {

        int i_count = 0;
        for (int j = 0; j < graph.size_free; ++j) {
            if (i != j) {
                if (getVariableValue(model, res, variable_matrix[j][i])) {
                    i_count++;
                }
            }
        }

        solution[i_count] = i;
    }

    return Order(solution);
}

#endif // PACE2024_OPTIMIZATION_SOLVER_HPP
