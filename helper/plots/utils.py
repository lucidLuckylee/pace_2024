import os

import pandas as pd


def parse_files_to_plot(unknown_args):
    file_names = {}
    last_arg = None

    for arg in unknown_args:
        if arg.startswith("--"):
            if last_arg is not None:
                print(f"Invalid argument {last_arg} does not have a name.")
                exit(1)
            arg = arg[2:]
            last_arg = arg
        else:
            if last_arg is None:
                print(f"Invalid argument {arg} does not have a file.")
                exit(1)
            file_names[last_arg] = arg
            last_arg = None

    if last_arg is not None:
        print(f"Invalid argument {last_arg} does not have a name.")
        exit(1)

    return file_names


def load_min_crossings_from_path(solutions_path):
    dataframes = {file_name: pd.read_csv(os.path.join(solutions_path, file_name)) for file_name in
                  os.listdir(solutions_path)}

    combined = pd.concat(dataframes)
    min_crossings = combined.groupby('file')["crossings"].min().reset_index()

    return min_crossings
