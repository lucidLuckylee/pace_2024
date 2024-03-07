from enum import Enum, auto
import os
import subprocess
import resource
import sys
import time
import argparse
import pace2024_verifier.pace as pace


SOLUTION_PATH = "output.txt"


class Status(Enum):
    OK = "OK"
    PROCESS_ERROR = "PROCESS ERROR"
    TIMEOUT = "TIMED OUT"
    OUT_OF_MEMORY = "OUT OF MEMORY"
    COMMAND_ERROR = "COMMAND FAILED"


def limit_virtual_memory(mem_limit_gb=None):
    # The tuple below is of the form (soft limit, hard limit). Limit only
    # the soft part so that the limit can be increased later (setting also
    # the hard limit would prevent that).
    # When the limit cannot be changed, setrlimit() raises ValueError.
    if mem_limit_gb is None:
        virtual_memory_limit = resource.RLIM_INFINITY
    else:
        virtual_memory_limit = mem_limit_gb * 1024 ** 3
    resource.setrlimit(resource.RLIMIT_AS,
                       (virtual_memory_limit, virtual_memory_limit))


def run_command_with_limit(cmd, input_path, output_path, timeout, mem_limit_gb=None):
    """
    Run a command with a time limit and redirecting stdin from a file.

    cmd: string, the command to run
    input_path: string, path to the file for stdin redirection
    timeout: int, time limit in seconds

    Returns: tuple containing return code, execution time, stderr,
             and a Status Enum indicating the process status after termination
    """

    start_time = time.time()
    status = None
    cmd = cmd.split(" ")
    pace_graph_file = open(input_path, 'r')
    output_file = open(output_path, 'w')

    try:
        process = subprocess.Popen(
            cmd,
            stdin=pace_graph_file,
            stdout=output_file,
            stderr=subprocess.PIPE,
            preexec_fn=limit_virtual_memory(mem_limit_gb)
        )

        try:
            return_code = process.wait(timeout)
            if return_code == 0:
                status = Status.OK
            else:
                status = Status.PROCESS_ERROR
        except subprocess.TimeoutExpired:
            process.terminate()
            return_code = process.wait()
            status = Status.TIMEOUT
        end_time = time.time()
        stderr = process.stderr.read().decode('utf-8')

    except MemoryError as e:
        end_time = time.time()
        stderr = str(e)
        return_code = -1
        status = Status.OUT_OF_MEMORY

    except subprocess.CalledProcessError as e:
        # This block will be entered if the command fails
        end_time = time.time()
        stderr = e.stderr.decode('utf-8') if e.stderr else ''
        return_code = e.returncode
        status = Status.COMMAND_ERROR

    pace_graph_file.close()
    output_file.close()

    return return_code, end_time - start_time, stderr, status


def count_crossings(input_path, solution_str):
    order = pace.read_solution(solution_str)
    pace_graph = pace.read_graph(input_path, order=order)
    crossings = pace_graph.countcrossings_segtree()
    return crossings


def check_if_solution_is_valid(graph_path, solution_path):
    current_used_nodes = set()
    order = []

    with open(graph_path, 'r') as f:
        line = f.readline()
        info = line.split(" ")
        fixed_size = int(info[2])
        free_size = int(info[3])

    with open(solution_path, 'r') as f:
        for i, line in enumerate(f):
            line = line.split("#")[0].strip()
            if len(line) == 0:
                continue

            try:
                vertex = int(line)
                if vertex < fixed_size or vertex > free_size + fixed_size:
                    print(f"Vertex {vertex} is out of range")
                    return

                if vertex in current_used_nodes:
                    print(f"Vertex {vertex} is already used")
                    return

                current_used_nodes.add(vertex)
                order.append(vertex - 1)

            except ValueError:
                print(f"Can not parse vertex {i}: ", line)
                return

    if len(current_used_nodes) != free_size:
        print(
            f"Only {len(current_used_nodes)} nodes are used, but {free_size} are required")
        return


def clean_output(output_path):
    with open(output_path, 'r') as output_file:
        lines = output_file.readlines()

    with open(output_path, 'w') as output_file:
        for line in lines:
            line = line.split("#")[0].strip()
            if len(line) == 0:
                continue

            if "Set parameter Username" in line or "Academic license - for non-commercial use only -" in line:
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
    parser.add_argument("--memlimit", type=int, default=4,
                        help="The memory limit for each run in GB. Default is 4.")
    parser.add_argument("-p", "--print", action="store_true", help="Print the result in terminal")

    args = parser.parse_args()
    base_path = args.base_path
    program = args.program
    paths = os.listdir(base_path)

    try:
        paths = sorted(paths, key=lambda x: int(x.split('.')[0]))
    except ValueError:
        pass

    if args.print:
        print(f"{'File':<16}{'Exit Code':<16}{'Time':<16}{'Status':<16}{'Crossings':<16}")
    else:
        print("file, exit_code, time, status, crossings")
    for path in paths:
        if path.endswith(".gr"):
            pace_graph_path = os.path.join(base_path, path)
            process_result = run_command_with_limit(program, pace_graph_path, SOLUTION_PATH, args.timelimit, mem_limit_gb=args.memlimit)
            return_code, time_delta, stderr, status = process_result

            if return_code == 0:
                clean_output(SOLUTION_PATH)
                check_if_solution_is_valid(pace_graph_path, SOLUTION_PATH)
                crossing = count_crossings(pace_graph_path, SOLUTION_PATH)
                if args.print:
                    print(f"{path:<16}{return_code:<16}{time_delta:<16.5f}{status.value:<16}{crossing:<16}")
                else:
                    print(path, return_code, time_delta, status.value, crossing, sep=",")

            else:
                if args.print:
                    print(f"{path:<16}{return_code:<16}{time_delta:<16.5f}{status.value:<16}{'':<16}")
                else:
                    print(path, return_code, time_delta, status.value, '', sep=",")
            sys.stdout.flush()


if __name__ == '__main__':
    main()
