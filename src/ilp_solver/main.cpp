#include "../pace_graph/pace_graph.hpp"

#include "ortools/linear_solver/linear_solver.h"

int main() {
    PaceGraph graph_from_file = PaceGraph::from_file("../data/tree_6_10.gr");

    std::unique_ptr<operations_research::MPSolver> solver(
        operations_research::MPSolver::CreateSolver("SCIP"));
    if (!solver) {
        LOG(WARNING) << "SCIP solver unavailable.";
        return 0;
    }

    std::vector<std::vector<operations_research::MPVariable *>> variable_matrix(
        graph_from_file.size_free);
    for (int i = 0; i < graph_from_file.size_free; ++i) {
        variable_matrix[i].resize(graph_from_file.size_free);
        for (int j = 0; j < graph_from_file.size_free; ++j) {
            variable_matrix[i][j] = solver->MakeBoolVar(
                "m_" + std::to_string(i) + "_" + std::to_string(j));
        }
    }

    auto const objective = solver->MutableObjective();

    for (int i = 0; i < graph_from_file.size_free; ++i) {
        for (int j = 0; j < graph_from_file.size_free; ++j) {
            if (i != j) {
                auto varij = variable_matrix[i][j];
                auto varji = variable_matrix[j][i];

                objective->SetCoefficient(
                    varij, graph_from_file.crossing_matrix[i][j]);

                auto *symmetry_constraint = solver->MakeRowConstraint(1, 1);
                symmetry_constraint->SetCoefficient(varij, 1);
                symmetry_constraint->SetCoefficient(varji, 1);

                for (int k = 0; k < graph_from_file.size_free; ++k) {
                    if (j != k && i != k) {
                        auto varjk = variable_matrix[j][k];
                        auto varik = variable_matrix[i][k];

                        auto *transitivity_constraint =
                            solver->MakeRowConstraint(-solver->infinity(), 1);
                        transitivity_constraint->SetCoefficient(varij, 1);
                        transitivity_constraint->SetCoefficient(varjk, 1);
                        transitivity_constraint->SetCoefficient(varik, -1);
                    }
                }
            }
        }
    }
    objective->SetMinimization();

    auto result_status = solver->Solve();
    if (result_status != operations_research::MPSolver::OPTIMAL) {
        std::cerr << "The problem does not have an optimal solution!";
    }

    std::cout << "#Crossings " << objective->Value() << std::endl;

    int *solution = new int[graph_from_file.size_free];

    for (int i = 0; i < graph_from_file.size_free; ++i) {

        int i_count = 0;
        for (int j = 0; j < graph_from_file.size_free; ++j) {
            if (i != j) {
                if (variable_matrix[j][i]->solution_value() > 0.5) {
                    i_count++;
                }
            }
        }

        solution[i_count] = i + graph_from_file.size_fixed + 1;
    }

    for (int i = 0; i < graph_from_file.size_free; ++i) {
        std::cout << solution[i] << std::endl;
    }

    delete[] solution;

    return 0;
}