import os

import matplotlib.pyplot as plt
import argparse

import pandas as pd

from plots.utils import parse_files_to_plot, load_min_crossings_from_path


def plot_cactus(solutions_path_heuristic, solutions_path_lb, file_names, output_file):
    plt.rcParams.update({'font.size': 22})
    plt.rc('text', usetex=True)
    plt.figure(figsize=(16, 10))

    min_crossings = load_min_crossings_from_path(solutions_path_heuristic)

    for file_name, name in file_names.items():
        df = pd.read_csv(os.path.join(solutions_path_lb, file_name))

        merged_df = pd.merge(df, min_crossings, on="file", suffixes=('_df', '_min'))
        df["diff"] = merged_df["crossings_min"] - merged_df["crossings_df"]

        if df["diff"].min() < 0:
            print(df[df["diff"] < 0])
            print(f"File {file_name} has larger lb then ub.")
            exit(1)
        print(f"Number of instances directly solved by {name}:", len(df[df["diff"] == 0]))
        plt.plot(df["diff"].sort_values().tolist(), label=name)

    plt.xlabel("\#Instance")
    plt.ylabel("Difference")
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_file)
    plt.show()


def main():
    parser = argparse.ArgumentParser(description="Create a cactus plot ")
    parser.add_argument("solutions_heuristic", type=str,
                        help="The path to the directory containing the heuristic csv files.")
    parser.add_argument("solutions_lb", type=str,
                        help="The path to the directory containing the lb csv files.")
    parser.add_argument("output", type=str, help="Output file name")

    args, unknown_args = parser.parse_known_args()
    file_names = parse_files_to_plot(unknown_args)

    if not os.path.exists(args.solutions_heuristic):
        print(f"Solution path: {args.solutions_heuristic} does not exist")
        exit(1)

    if not os.path.exists(args.solutions_lb):
        print(f"Solution path: {args.solutions_lb} does not exist")
        exit(1)

    for file_name in file_names.keys():
        full_path = os.path.join(args.solutions_lb, file_name)
        if not os.path.exists(full_path):
            print(f"File path: {full_path} does not exist")
            exit(1)

    plot_cactus(args.solutions_heuristic, args.solutions_lb, file_names, args.output)


if __name__ == '__main__':
    main()
