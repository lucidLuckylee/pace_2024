import argparse

from test import clean_output, check_if_solution_is_valid, count_crossings

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Print the number of crossings for a specific file.")
    parser.add_argument("graph", type=str,
                        help="The path to the graph.")
    parser.add_argument("solution", type=str,
                        help="The path to the solution.")

    args = parser.parse_args()
    graph = args.graph
    solution = args.solution

    clean_output(solution)
    check_if_solution_is_valid(graph, solution)
    crossing = count_crossings(graph, solution)
    print(crossing)
