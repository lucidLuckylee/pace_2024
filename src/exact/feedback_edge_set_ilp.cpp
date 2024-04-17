#include "feedback_edge_set_ilp.hpp"
#include <ortools/linear_solver/linear_solver.h>

void solveFeedbackEdgeSetILP(FeedbackEdgeInstance &instance) {
    if (instance.ilpModel == nullptr) {
        std::string solverName = "SCIP";
        auto model = operations_research::MPSolver::CreateSolver(solverName);
        instance.ilpModel = model;
        model->SetNumThreads(1);
        if (solverName == "GUROBI") {
            model->SetSolverSpecificParametersAsString("Threads 1");
        }
    }

    auto solver =
        static_cast<operations_research::MPSolver *>(instance.ilpModel);

    auto *objective = solver->MutableObjective();
    objective->SetMinimization();

    for (auto &e : instance.usedEdges) {
        if (e->ilpVar == nullptr) {
            auto var = solver->MakeBoolVar(std::to_string(e->start) + "->" +
                                           std::to_string(e->end));
            e->ilpVar = var;
            objective->SetCoefficient(var, e->weight);
        }
    }

    for (auto &c : instance.circles) {
        if (c->ilpConstraint == nullptr) {
            auto *constraint = solver->MakeRowConstraint(1, c->edges.size());
            c->ilpConstraint = constraint;
            for (auto &e : c->edges) {
                constraint->SetCoefficient(
                    static_cast<operations_research::MPVariable *>(e->ilpVar),
                    1);
            }
        }
    }

    solver->Solve();
    long solutionSize = 0;
    instance.bestSolution.clear();
    for (auto &e : instance.usedEdges) {
        auto var = static_cast<operations_research::MPVariable *>(e->ilpVar);
        if (var->solution_value() > 0.5) {
            instance.bestSolution.emplace_back(e);
            solutionSize += e->weight;
        }
    }
    std::cerr << "Exact solution: " << solutionSize << "/" << instance.globalUB
              << std::endl;

    instance.ub = solutionSize;
}