import subprocess
import argparse
import os


def run_command(cmd, input_path, output_path):
    cmd = cmd.split(" ")
    pace_graph_file = open(input_path, 'r')
    output_file = open(output_path, 'w')

    subprocess.Popen(
        cmd,
        stdin=pace_graph_file,
        stdout=output_file,
        stderr=subprocess.PIPE,
    ).wait()

    pace_graph_file.close()
    output_file.close()


def main():
    parser = argparse.ArgumentParser(
        description="Run a program on files in a specified directory.")
    parser.add_argument("input_path", type=str,
                        help="The base path to the directory containing the gr files.")
    parser.add_argument("program", type=str,
                        help="The path to the program to be run.")
    parser.add_argument("output_path", type=str, help="Path for writing the reduced gr files to.")

    args = parser.parse_args()
    input_path = args.input_path
    output_path = args.output_path
    program = args.program
    paths = os.listdir(input_path)

    try:
        paths = sorted(paths, key=lambda x: int(x.split('.')[0]))
    except ValueError:
        pass

    for path in paths:
        if path.endswith(".gr"):
            pace_graph_path = os.path.join(input_path, path)
            pace_graph_output_path = os.path.join(output_path, path)
            run_command(program, pace_graph_path, pace_graph_output_path)


if __name__ == '__main__':
    main()
