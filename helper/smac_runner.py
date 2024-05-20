from ConfigSpace import ConfigurationSpace, UniformIntegerHyperparameter
from smac import Scenario, BlackBoxFacade, HyperparameterOptimizationFacade, AlgorithmConfigurationFacade

from wrapper_script import run_program


def objective_function(cfg, seed: int = 0):
    cfg = {k: cfg[k] for k in cfg if cfg[k]}
    result = run_program(cfg['forceMoveAllDirectNodesAfterIterationWithNoImprovement'],
                         cfg['numberOfForceSwapPositions'], cfg['numberOfForceSwapStepSize'])
    print(result)
    return result


cs = ConfigurationSpace()
cs.add_hyperparameter(UniformIntegerHyperparameter("forceMoveAllDirectNodesAfterIterationWithNoImprovement", 1, 1000))
cs.add_hyperparameter(UniformIntegerHyperparameter("numberOfForceSwapPositions", 1, 100))
cs.add_hyperparameter(UniformIntegerHyperparameter("numberOfForceSwapStepSize", 1, 10))

scenario = Scenario(cs, deterministic=True)

smac = HyperparameterOptimizationFacade(
    scenario,
    objective_function,
)
incumbent = smac.optimize()
best_cfg = smac.optimize()
print(best_cfg)
