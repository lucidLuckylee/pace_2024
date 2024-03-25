import os

import matplotlib.pyplot as plt
import argparse

import pandas as pd

from plots.utils import parse_files_to_plot


def load_exact_solutions_from_path(solutions_path):
    dataframes = {file_name: pd.read_csv(os.path.join(solutions_path, file_name)) for file_name in
                  os.listdir(solutions_path)}

    combined = pd.concat(dataframes)
    min_crossings = combined.groupby('file')["crossings"].min().reset_index()

    for _, df in dataframes.items():
        merged_df = pd.merge(df, min_crossings, on="file", suffixes=('_df', '_min'))
        df["percentage"] = merged_df["crossings_df"] / merged_df["crossings_min"]
    return dataframes


def plot_cactus(solutions_path, file_names, output_file):
    plt.rcParams.update({'font.size': 22})
    plt.rc('text', usetex=True)
    plt.figure(figsize=(16, 10))

    dataframes = load_exact_solutions_from_path(solutions_path)

    for file_name, name in file_names.items():
        df = dataframes[file_name]
        y_values = df["percentage"].sort_values().tolist()
        plt.plot(y_values, label=name)

    plt.xlabel("\#Instance")
    plt.ylabel("Percentage")

    plt.yscale('log')
    plt.legend()

    plt.tight_layout()
    plt.savefig(output_file)
    plt.show()


def main():
    parser = argparse.ArgumentParser(description="Create a cactus plot ")
    parser.add_argument("solutions", type=str, help="The path to the directory containing the heuristic csv files.")
    parser.add_argument("output", type=str, help="Output file name")

    args, unknown_args = parser.parse_known_args()
    file_names = parse_files_to_plot(unknown_args)

    if not os.path.exists(args.solutions):
        print(f"Solution path: {args.solutions} does not exist")
        exit(1)

    for file_name in file_names.keys():
        full_path = os.path.join(args.solutions, file_name)
        if not os.path.exists(full_path):
            print(f"File path: {full_path} does not exist")
            exit(1)

    plot_cactus(args.solutions, file_names, args.output)


if __name__ == '__main__':
    main()
