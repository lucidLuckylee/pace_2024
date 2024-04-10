import argparse
import os

from test import run_command_with_limit, SOLUTION_PATH


def check_if_solution_is_valid(graph, solution):
    edge_weights = []
    circles = []

    with open(graph, 'r') as graph_file:
        for line in graph_file:
            if line.startswith("p"):
                [_, n, m, c, __, exact] = line.split(" ")
                n = int(n)
                m = int(m)
                c = int(c)
                exact = int(exact)
                for i in range(n):
                    edge_weights.append({})
            else:
                if m > 0:
                    [u, v, w] = line.split(" ")
                    edge_weights[int(u)][int(v)] = int(w)
                    m -= 1
                elif c > 0:
                    edges = line.split(" ")
                    for i in range(len(line), 0, 2):
                        u = int(edges[i])
                        v = int(edges[i + 1])
                        circles.append((u, v))

    sol = set()
    with open(solution, 'r') as solution_file:
        for line in solution_file:
            [u, v] = line.split(" ")
            sol.add((int(u), int(v)))

    for circle in circles:
        covered = False
        for (u, v) in circle:
            if (u, v) in sol or (v, u) in sol:
                covered = True
                break
        if not covered:
            print(f"Circle {circle} is not covered.")
            return 0

    weight = 0
    for (u, v) in sol:
        weight += edge_weights[u][v]

    return weight, exact


def clean_output(output_path):
    with open(output_path, 'r') as output_file:
        lines = output_file.readlines()

    with open(output_path, 'w') as output_file:
        for line in lines:

            line = line.split("#")[0].strip()
            if len(line) == 0:
                continue
            output_file.write(line)
            output_file.write("\n")


def main():
    parser = argparse.ArgumentParser(
        description="Run a program on files in a specified directory.")

    parser.add_argument("base_path", type=str,
                        help="The base path to the directory containing the test files.")
    parser.add_argument("program", type=str,
                        help="The path to the program to be run.")

    parser.add_argument("--timelimit", type=int, default=300,
                        help="The time limit for each run in seconds. Default is 300.")
    parser.add_argument("--memlimit", type=int, default=8,
                        help="The memory limit for each run in GB. Default is 8.")

    args = parser.parse_args()
    base_path = args.base_path
    program = args.program
    paths = os.listdir(base_path)

    try:
        paths = sorted(paths, key=lambda x: int(x.split('.')[0]))
    except ValueError:
        pass

    print("file", "exit_code", "time", "status", "weight", "exact", sep=",")
    for path in paths:
        if path.endswith(".gr"):
            pace_graph_path = os.path.join(base_path, path)
            return_code, time_delta, stderr, status = run_command_with_limit(program, pace_graph_path, SOLUTION_PATH,
                                                                             args.timelimit,
                                                                             mem_limit_gb=args.memlimit)
            if return_code == 0:
                clean_output(SOLUTION_PATH)
                weight, exact = check_if_solution_is_valid(pace_graph_path, SOLUTION_PATH)
                print(path, return_code, time_delta, status.value, weight, exact, sep=",")
            else:
                print(path, return_code, time_delta, status.value, "", "", sep=",")


if __name__ == '__main__':
    main()
