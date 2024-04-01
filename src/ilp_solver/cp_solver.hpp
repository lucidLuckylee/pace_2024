//
// Created by jt on 27.03.24.
//

#ifndef PACE2024_CP_SOLVER_HPP
#define PACE2024_CP_SOLVER_HPP

#include "optimization_solver.hpp"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"

class CPSolver
    : public OptimizationSolver<operations_research::sat::CpModelBuilder,
                                operations_research::sat::BoolVar,
                                operations_research::sat::LinearExpr,
                                operations_research::sat::CpSolverResponse> {
  protected:
    operations_research::sat::CpModelBuilder createModel() override {
        return {};
    }
    operations_research::sat::LinearExpr createOptFunctions(
        operations_research::sat::CpModelBuilder &model) override {
        return {};
    }
    void addOptFunctions(operations_research::sat::CpModelBuilder &model,
                         operations_research::sat::LinearExpr optFun) override {
        model.Minimize(optFun);
    }
    operations_research::sat::BoolVar
    createVariable(operations_research::sat::CpModelBuilder &model,
                   std::string name) override {
        return model.NewBoolVar().WithName(name);
    }
    void
    addSymmetryConstraints(operations_research::sat::CpModelBuilder &model,
                           operations_research::sat::BoolVar &var1,
                           operations_research::sat::BoolVar &var2) override {

        model.AddBoolOr({var1, var2});
        model.AddLessOrEqual(
            operations_research::sat::LinearExpr::Sum({var1, var2}), 1);
    }
    void
    addTransitiveConstraints(operations_research::sat::CpModelBuilder &model,
                             operations_research::sat::BoolVar &var1,
                             operations_research::sat::BoolVar &var2,
                             operations_research::sat::BoolVar &var3) override {
        model.AddLinearConstraint(var1 + var2 - var3,
                                  operations_research::Domain(0, 1));
    }
    void setVariable(operations_research::sat::CpModelBuilder &model,
                     operations_research::sat::BoolVar &var,
                     bool val) override {
        model.AddEquality(var, val);
    }

    void setCoefficient(operations_research::sat::CpModelBuilder &model,
                        operations_research::sat::LinearExpr &optFun,
                        operations_research::sat::BoolVar &var,
                        int crossingMatrix) override {
        optFun += crossingMatrix * var;
    }
    operations_research::sat::CpSolverResponse
    solveModel(operations_research::sat::CpModelBuilder &model) override {
        return Solve(model.Build());
    }
    bool getVariableValue(operations_research::sat::CpModelBuilder &mode,
                          operations_research::sat::CpSolverResponse &result,
                          operations_research::sat::BoolVar &var) override {
        return operations_research::sat::SolutionBooleanValue(result, var);
    }
};
#endif // PACE2024_CP_SOLVER_HPP
