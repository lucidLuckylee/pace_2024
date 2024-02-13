import os
import subprocess
import time
import argparse


def run_command_with_limit(cmd, input_file, timeout, mem_limit_gb=None):
    """
    Run a command with a time limit and redirecting stdin from a file.

    cmd: string, the command to run
    input_file: string, path to the file for stdin redirection
    timeout: int, time limit in seconds

    Returns: tuple containing return code, execution time, stdout, stderr,
             and a boolean indicating if the process was terminated due to a timeout
    """

    start_time = time.time()

    try:

        with open(input_file, 'r') as f, subprocess.Popen(["bash", "-c", cmd], stdin=f, stdout=subprocess.PIPE,
                                                          stderr=subprocess.PIPE) as process:

            if mem_limit_gb is not None:
                import psutil
                mem_limit_kb = mem_limit_gb * 1024 * 1024
                ps_process = psutil.Process(process.pid)
                while process.poll() is None:
                    time.sleep(1)
                    if ps_process.memory_info().vms > mem_limit_kb * 1024:
                        process.terminate()
                        raise MemoryError("Process exceeded memory limit")

                    if time.time() - start_time > timeout:
                        process.terminate()
                        raise subprocess.TimeoutExpired(process.args, timeout)
            process.wait()
            stdout = "".join([s.decode('utf-8') for s in process.stdout.readlines()])
            stderr = "".join([s.decode('utf-8') for s in process.stderr.readlines()])

            return_code = process.returncode

            end_time = time.time()
            was_timeout = False

    except subprocess.TimeoutExpired:
        # This block will be entered if the command times out
        end_time = time.time()
        stdout = ''
        stderr = ''
        return_code = ''
        was_timeout = True
    except MemoryError as e:
        end_time = time.time()
        stdout = ''
        stderr = str(e)
        return_code = -1
        was_timeout = False
    except subprocess.CalledProcessError as e:
        # This block will be entered if the command fails
        end_time = time.time()
        stdout = e.stdout.decode('utf-8') if e.stdout else ''
        stderr = e.stderr.decode('utf-8') if e.stderr else ''
        return_code = e.returncode
        was_timeout = False

    except Exception as e:
        # This block will be entered if an unexpected exception occurs
        end_time = time.time()
        stdout = ''
        stderr = str(e)
        return_code = -1
        was_timeout = False

    return return_code, end_time - start_time, stdout, stderr, was_timeout


class Graph:
    def __init__(self, fixed_size, free_size):
        self.fixed_size = fixed_size
        self.free_size = free_size
        self.edges_fixed = [[] for _ in range(fixed_size)]
        self.edges_free = [[] for _ in range(free_size)]

    def add_edge(self, a, b):
        self.edges_fixed[a].append(b)
        self.edges_free[b - self.fixed_size].append(a)


def get_solution(graph: Graph, solution_str: str):
    current_used_nodes = set()
    order = []
    for i, line in enumerate(solution_str.split("\n")):
        line = line.split("#")[0].strip()
        if len(line) == 0:
            continue

        if "Set parameter Username" in line or "Academic license - for non-commercial use only -" in line:
            continue

        try:
            vertex = int(line)
            if vertex < graph.fixed_size or vertex > graph.free_size + graph.fixed_size:
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

    if len(current_used_nodes) != graph.free_size:
        print(f"Only {len(current_used_nodes)} nodes are used, but {graph.free_size} are required")
        return None

    return order


def load_graph(input_path):
    with open(input_path, 'r') as f:
        line = f.readline()
        infos = line.split(" ")
        fixed_size = int(infos[2])
        free_size = int(infos[3])
        g = Graph(fixed_size, free_size)

        for line in f.readlines():
            if len(line.strip()) == 0:
                continue

            edge = line.split(" ")
            a = int(edge[0])
            b = int(edge[1])
            g.add_edge(a - 1, b - 1)
    return g


def count_crossings(input_path, solution_str):
    g = load_graph(input_path)
    order = get_solution(g, solution_str)
    if order is None:
        return None

    crossings = 0
    for free_i in range(len(order)):
        for free_j in range(free_i + 1, len(order)):
            neighbor_i = g.edges_free[order[free_i] - g.fixed_size]
            neighbor_j = g.edges_free[order[free_j] - g.fixed_size]

            for i in neighbor_i:
                for j in neighbor_j:
                    if i > j:
                        crossings += 1

    return crossings


def main():
    parser = argparse.ArgumentParser(description="Run a program on files in a specified directory.")
    parser.add_argument("base_path", type=str, help="The base path to the directory containing the test files.")
    parser.add_argument("program", type=str, help="The path to the program to be run.")

    args = parser.parse_args()

    base_path = args.base_path
    program = args.program

    files = os.listdir(base_path)
    files = sorted(files, key=lambda x: int(x.split('.')[0]))
    print("file,time,crossings")
    for f in files:
        if f.endswith(".gr"):
            path = os.path.join(base_path, f)
            return_code, time_delta, stdout, stderr, timeout = run_command_with_limit(program, path, 300)
            if return_code == 0 and not timeout:
                crossing = count_crossings(path, stdout)
                print(f, time_delta, crossing, sep=",")
            else:
                print(f, "", "", sep=",")


if __name__ == '__main__':
    main()
