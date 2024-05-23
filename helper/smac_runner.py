from ConfigSpace import ConfigurationSpace, UniformIntegerHyperparameter
from smac import Scenario, HyperparameterOptimizationFacade

from wrapper_script import run_program


def objective_function(cfg, seed: int = 0, num_evals=5):
    cfg = {k: cfg[k] for k in cfg if cfg[k]}

    results = []

    print("Test parameters: ", cfg)
    for i in range(num_evals):
        result = run_program(cfg['forceMoveAllDirectNodesAfterIterationWithNoImprovement'],
                             cfg['numberOfForceSwapPositions'], cfg['numberOfForceSwapStepSize'])

        print(f"Result {i}: {result} for {cfg}")
        results.append(result)
    average_result = sum(results) / len(results)
    print(f"Finish {cfg} with average result: ", average_result)
    return average_result


cs = ConfigurationSpace()
cs.add_hyperparameter(UniformIntegerHyperparameter("forceMoveAllDirectNodesAfterIterationWithNoImprovement", 1, 1000))
cs.add_hyperparameter(UniformIntegerHyperparameter("numberOfForceSwapPositions", 1, 100))
cs.add_hyperparameter(UniformIntegerHyperparameter("numberOfForceSwapStepSize", 1, 10))

scenario = Scenario(cs, n_workers=20, n_trials=500)

smac = HyperparameterOptimizationFacade(
    scenario,
    objective_function,
)
incumbent = smac.optimize()
print(incumbent)
