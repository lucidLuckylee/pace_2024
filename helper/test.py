import os
import re
import subprocess
import resource
import sys
import time
import argparse
import pace2024_verifier.pace as pace


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


def run_command_with_limit(cmd, input_file, output_file, timeout, mem_limit_gb=None):
    """
    Run a command with a time limit and redirecting stdin from a file.

    cmd: string, the command to run
    input_file: string, path to the file for stdin redirection
    timeout: int, time limit in seconds

    Returns: tuple containing return code, execution time, stdout, stderr,
             and a boolean indicating if the process was terminated due to a timeout
    """

    start_time = time.time()
    status = None
    try:
        cmd = cmd.split(" ")
        with open(input_file, 'r') as f, open(output_file, 'w') as out, subprocess.Popen(cmd, stdin=f, stdout=out,
                                                                                         stderr=subprocess.PIPE,
                                                                                         preexec_fn=limit_virtual_memory(
                                                                                             mem_limit_gb)) as process:
            try:
                return_code = process.wait(timeout)
            except subprocess.TimeoutExpired:
                process.terminate()
                return_code = process.wait()
            end_time = time.time()
            stderr = process.stderr.read().decode('utf-8')
            if return_code == 0:
                status = "ok"
            else:
                status = "Program exited with non zero exit code"

    except subprocess.TimeoutExpired:
        # This block will be entered if the command times out
        end_time = time.time()
        stderr = ''
        return_code = ''
        status = "timeout"
    except MemoryError as e:
        end_time = time.time()
        stderr = str(e)
        return_code = -1
        status = "memory limit"
    except subprocess.CalledProcessError as e:
        # This block will be entered if the command fails
        end_time = time.time()
        stderr = e.stderr.decode('utf-8') if e.stderr else ''
        return_code = e.returncode
        status = "error"

    except Exception as e:
        # This block will be entered if an unexpected exception occurs
        end_time = time.time()
        stderr = str(e)
        return_code = -1
        status = "error"

    return return_code, end_time - start_time, stderr, status


def count_crossings(input_path, solution_str):
    order = pace.read_solution(solution_str)
    g = pace.read_graph(input_path, order=order)
    crossings = g.countcrossings_segtree()
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
                    return None

                if vertex in current_used_nodes:
                    print(f"Vertex {vertex} is already used")
                    return None

                current_used_nodes.add(vertex)
                order.append(vertex - 1)

            except ValueError:
                print(f"Can not parse vertex {i}: ", line)
                return None

    if len(current_used_nodes) != free_size:
        print(
            f"Only {len(current_used_nodes)} nodes are used, but {free_size} are required")
        return None


def clean_output(output_path):
    with open(output_path, 'r') as f:
        lines = f.readlines()

    iterations = ""

    with open(output_path, 'w') as f:
        for line in lines:
            match = re.search(r"#\s*Iterations\s*:\s*(\d+)", line)
            if match:
                iterations = str(match.group(1))
                continue
            line = line.split("#")[0].strip()
            if len(line) == 0:
                continue

            if "Set parameter Username" in line or "Academic license - for non-commercial use only -" in line:
                continue

            f.write(line)
            f.write("\n")
    return iterations


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
                        help="The memory limit for each run in GB. Default is 8.")

    args = parser.parse_args()

    base_path = args.base_path
    program = args.program

    files = os.listdir(base_path)
    try:
        files = sorted(files, key=lambda x: int(x.split('.')[0]))
    except ValueError:
        pass
    print("file", "exit_code", "time", "status", "crossings", "iterations", sep=",")
    for f in files:
        if f.endswith(".gr"):
            path = os.path.join(base_path, f)
            solution_path = "output.txt"
            return_code, time_delta, stderr, status = run_command_with_limit(program, path, solution_path,
                                                                             args.timelimit,
                                                                             mem_limit_gb=args.memlimit)

            if return_code == 0:
                iterations = clean_output(solution_path)
                check_if_solution_is_valid(path, solution_path)
                crossing = count_crossings(path, solution_path)
                print(f, return_code, time_delta, status, crossing, iterations, sep=",")
            else:
                print(f, return_code, time_delta, status, "", "", sep=",")
            sys.stdout.flush()


if __name__ == '__main__':
    main()
