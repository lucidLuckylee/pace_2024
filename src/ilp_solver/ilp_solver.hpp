//
// Created by jt on 01.03.24.
//

#ifndef PACE2024_ILP_SOLVER_HPP
#define PACE2024_ILP_SOLVER_HPP

#include "optimization_solver.hpp"
#include <ortools/linear_solver/linear_solver.h>

class ILPSolver
    : public OptimizationSolver<operations_research::MPSolver *,
                                operations_research::MPVariable *,
                                operations_research::MPObjective *,
                                operations_research::MPSolver::ResultStatus> {

  protected:
    operations_research::MPSolver *createModel() override {
        std::string solver_name = "GUROBI";
        return operations_research::MPSolver::CreateSolver(solver_name);
    }

    operations_research::MPVariable *
    createVariable(operations_research::MPSolver *&model,
                   std::string name) override {
        return model->MakeBoolVar(name);
    }
    void
    addSymmetryConstraints(operations_research::MPSolver *&model,
                           operations_research::MPVariable *&var1,
                           operations_research::MPVariable *&var2) override {
        auto *symmetry_constraint = model->MakeRowConstraint(1, 1);
        symmetry_constraint->SetCoefficient(var1, 1);
        symmetry_constraint->SetCoefficient(var2, 1);
    }
    void
    addTransitiveConstraints(operations_research::MPSolver *&model,
                             operations_research::MPVariable *&var1,
                             operations_research::MPVariable *&var2,
                             operations_research::MPVariable *&var3) override {
        auto *transitivity_constraint = model->MakeRowConstraint(0, 1);
        transitivity_constraint->SetCoefficient(var1, 1);
        transitivity_constraint->SetCoefficient(var2, 1);
        transitivity_constraint->SetCoefficient(var3, -1);
    }
    void setVariable(operations_research::MPSolver *&model,
                     operations_research::MPVariable *&var, bool val) override {
        var->SetBounds(val, val);
    }

    void setCoefficient(operations_research::MPSolver *&model,
                        operations_research::MPObjective *&optFun,
                        operations_research::MPVariable *&var,
                        int crossingMatrix) override {
        optFun->SetCoefficient(var, crossingMatrix);
    }

    operations_research::MPObjective *
    createOptFunctions(operations_research::MPSolver *&model) override {
        return model->MutableObjective();
    }

    void addOptFunctions(operations_research::MPSolver *&model,
                         operations_research::MPObjective *optFun) override {
        optFun->SetMinimization();
    }

    bool getVariableValue(operations_research::MPSolver *&mode,
                          operations_research::MPSolver::ResultStatus &result,
                          operations_research::MPVariable *&var) override {
        return var->solution_value() > 0.5;
    }

    operations_research::MPSolver::ResultStatus
    solveModel(operations_research::MPSolver *&model) override {
        return model->Solve();
    }

    void setThreadLimit(operations_research::MPSolver *&model,
                        int limit) override {
        model->SetNumThreads(limit);
        model->SetSolverSpecificParametersAsString("Threads " +
                                                   std::to_string(limit));
    }

  public:
    explicit ILPSolver(
        std::chrono::milliseconds limit = std::chrono::milliseconds::max())
        : OptimizationSolver(limit) {}
};

#endif // PACE2024_ILP_SOLVER_HPP
