//
// Created by jt on 01.03.24.
//

#include "ilp_solver.hpp"
#include <ortools/linear_solver/linear_solver.h>

Order ilpSolver(PaceGraph &graph) {
    graph.init_crossing_matrix_if_necessary();
    auto solver_name = "GUROBI";

    std::unique_ptr<operations_research::MPSolver> solver(
        operations_research::MPSolver::CreateSolver(solver_name));
    if (!solver) {
        LOG(WARNING) << solver_name << " solver unavailable.";
        return Order(graph.size_free);
    }

    std::vector<std::vector<operations_research::MPVariable *>> variable_matrix(
        graph.size_free);
    for (int i = 0; i < graph.size_free; ++i) {
        variable_matrix[i].resize(graph.size_free);
        for (int j = 0; j < graph.size_free; ++j) {
            variable_matrix[i][j] = solver->MakeBoolVar(
                "m_" + std::to_string(i) + "_" + std::to_string(j));
        }
    }

    auto const objective = solver->MutableObjective();

    for (int i = 0; i < graph.size_free; ++i) {
        for (int j = 0; j < graph.size_free; ++j) {
            if (i != j) {
                auto varij = variable_matrix[i][j];
                auto varji = variable_matrix[j][i];

                objective->SetCoefficient(varij, graph.crossing_matrix[i][j]);

                auto *symmetry_constraint = solver->MakeRowConstraint(1, 1);
                symmetry_constraint->SetCoefficient(varij, 1);
                symmetry_constraint->SetCoefficient(varji, 1);
            }
        }
    }
    objective->SetMinimization();
    bool is_transitivity;
    int transitivity_constraints = 0;
    do {
        solver->Solve();
        is_transitivity = true;

        std::vector<std::tuple<operations_research::MPVariable *,
                               operations_research::MPVariable *,
                               operations_research::MPVariable *>>
            transitivity_violations;

        for (int i = 0; i < graph.size_free; ++i) {
            for (int j = 0; j < graph.size_free; ++j) {
                for (int k = 0; k < graph.size_free; ++k) {
                    if (j != k && i != k) {
                        auto varij = variable_matrix[i][j];
                        auto varjk = variable_matrix[j][k];
                        auto varik = variable_matrix[i][k];

                        if (varij->solution_value() > 0.5 &&
                            varjk->solution_value() > 0.5 &&
                            varik->solution_value() < 0.5) {

                            transitivity_violations.emplace_back(varij, varjk,
                                                                 varik);
                            transitivity_constraints++;
                            is_transitivity = false;
                        }
                    }
                }
            }
        }

        for (const auto &violation : transitivity_violations) {
            operations_research::MPVariable *varij, *varjk, *varik;
            std::tie(varij, varjk, varik) = violation;

            auto *transitivity_constraint =
                solver->MakeRowConstraint(-solver->infinity(), 1);
            transitivity_constraint->SetCoefficient(varij, 1);
            transitivity_constraint->SetCoefficient(varjk, 1);
            transitivity_constraint->SetCoefficient(varik, -1);
        }

    } while (!is_transitivity);

    std::cout << "# Transitivity constraints added: "
              << transitivity_constraints << std::endl;

    std::vector<int> solution(graph.size_free);

    for (int i = 0; i < graph.size_free; ++i) {

        int i_count = 0;
        for (int j = 0; j < graph.size_free; ++j) {
            if (i != j) {
                if (variable_matrix[j][i]->solution_value() > 0.5) {
                    i_count++;
                }
            }
        }

        solution[i_count] = i;
    }

    return Order(solution);
}
