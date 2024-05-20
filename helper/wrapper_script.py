import os
import sys

from test import run_command_with_limit, SOLUTION_PATH, clean_output, check_if_solution_is_valid, count_crossings


def run_program(param1, param2, param3):
    base_path = "/home/jt/work/uni/pace/pace_2024/data/heuristic_public/"
    program = f"/home/jt/work/uni/pace/pace_2024/cmake-build-release/heuristic_solver {param1} {param2} {param3}"
    paths = os.listdir(base_path)

    try:
        paths = ["13.gr", "19.gr", "24.gr", "37.gr", "38.gr", "55.gr", "68.gr", "73.gr", "89.gr", "94.gr"]
        optimal_sol = [3361112, 11031590, 4450937, 33588854, 30602538, 1282785, 1282785, 567661, 2833852, 365618,
                       3599100]
    except ValueError:
        pass

    points = 0

    for i, path in enumerate(paths):
        pace_graph_path = os.path.join(base_path, path)
        run_command_with_limit(program, pace_graph_path, SOLUTION_PATH,
                               300,
                               mem_limit_gb=8)
        clean_output(SOLUTION_PATH)
        check_if_solution_is_valid(pace_graph_path, SOLUTION_PATH)
        optimal = optimal_sol[i]
        weight = count_crossings(pace_graph_path, SOLUTION_PATH)
        points += 1 - (optimal / weight)

    return points
