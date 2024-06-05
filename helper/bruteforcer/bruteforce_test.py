from bruteforcer.random_graph_generator import generate_random_graph
from bruteforcer.simple_gurobi_solver import gurobi_solver
from test import run_command_with_limit, count_crossings, clean_output


def main():
    program = "../../cmake-build-release/feedback_edge_set_solver"
    pace_graph_path = 'tmp.gr'
    solution_path = 'tmp.sol'
    a = 0
    while True:
        edges_fixed, edges_free, lenA, lenB, crossing_matrix = generate_random_graph()

        nr_edges = 0
        for i in range(len(edges_fixed)):
            nr_edges += len(edges_fixed[i])

        with open(pace_graph_path, 'w') as f:
            f.write(f'p ocr {lenA} {lenB} {nr_edges}\n')
            for i, neig in enumerate(edges_fixed):
                for j in neig:
                    f.write(f'{i + 1} {j + 1 + lenA}\n')

        solution = gurobi_solver(lenA, lenB, crossing_matrix)

        with open(solution_path, 'w') as f:
            for node in solution:
                f.write(f"{node}\n")

        crossing = count_crossings(pace_graph_path, solution_path)
        return_code, time_delta, mem, stderr, status = run_command_with_limit(program, pace_graph_path, solution_path,
                                                                              300,
                                                                              mem_limit_gb=8)

        if return_code != 0:
            print(status)
            print(stderr)
            break
        else:
            clean_output(solution_path)
            crossing_algo = count_crossings(pace_graph_path, solution_path)
            if crossing_algo > crossing:
                print(f"Found sol {crossing_algo} but optimal is {crossing}")
                break
            else:
                a += 1
                if a % 10 == 0:
                    print(f"Tested {a} many solutions")


if __name__ == '__main__':
    main()
