import random
import platform
from enum import Enum
import re
import subprocess
import resource
import time


P_EDGE = 0.01


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
    output_file = open("/dev/null", 'w')
    error_file = open(output_path, 'w')
    mem = ""

    try:
        if platform.system() == 'Linux':
            cmd = ["/usr/bin/time", '-v'] + cmd
            cmd = ["timeout", f"{timeout + 1}s"] + cmd
        process = subprocess.Popen(
            cmd,
            stdin=pace_graph_file,
            stdout=output_file,
            stderr=error_file,
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
        with open("error.txt", 'r') as f:
            stderr = f.read()
            memory_usage_search = re.search(r"Maximum resident set size \(kbytes\): (\d+)", stderr)
            if memory_usage_search:
                memory_usage_kb = int(memory_usage_search.group(1))
                mem = memory_usage_kb / (1024 ** 2)

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

    return return_code, end_time - start_time, mem, stderr, status


class PaceGraph:
    def __init__(self, a: int, b:int , edges: list[tuple[int, int]]):
        self.size_free = a
        self.size_fixed = b

        self.neighbors_fixed = [[] for _ in range(self.size_fixed)]
        self.neighbors_free = [[] for _ in range(self.size_free)]

        for (u, v) in edges:
            self.neighbors_free[u].append(v - self.size_free)
            self.neighbors_fixed[v - self.size_free].append(u)

        for neighbors in self.neighbors_fixed:
            neighbors.sort()

        for neighbors in self.neighbors_free:
            neighbors.sort()

    def __str__(self):
        result = [f"PaceGraph with {self.size_fixed} fixed vertices and {self.size_free} free vertices"]
        result.append("Neighbors (Fixed):")
        for i, neighbors in enumerate(self.neighbors_fixed):
            result.append(f"  Vertex {i}: {neighbors}")
        result.append("Neighbors (Free):")
        for i, neighbors in enumerate(self.neighbors_free):
            result.append(f"  Vertex {i}: {neighbors}")
        return "\n".join(result)

    def to_gr(self):
        num_edges = 0
        for neighbors in self.neighbors_fixed:
            num_edges += len(neighbors)
        result= f"p ocr {self.size_fixed} {self.size_free} {num_edges}\n"
        for i in range(0, self.size_fixed):
            for neighbor in self.neighbors_fixed[i]:
                result += f"{i+1} {neighbor + self.size_fixed + 1}\n"
        return result

    def rr2(self):
        pass


def neighborhood_borders(a, edges) -> tuple[int, int]:
    min = 0xFFFFFFF
    max = -1

    for u, v in edges:
        if u == a:
            if v < min:
                min = v
            if v > max:
                max = v
    return (min, max)


def random_rr2_graph(a: int, b: int, p_edge: float) -> PaceGraph:
    edges = []
    for i in range(a):
        for j in range(a, a + b):
            if random.random() < p_edge:
                edges.append((i, j))

    u_borders = (0xFFFFFFF, -1)
    v_borders = (0xFFFFFFF, -1)
    while (u_borders[0] == 0xFFFFFFF or u_borders[1] == -1 or v_borders[0] == 0xFFFFFFF or v_borders[1] == -1):
        # Pick two random vertices and craft same neighboorhood borders for them
        u = random.randint(0, a - 1)
        v = random.randint(0, a - 2)

        if u >= v:
            u+= 1
        u_borders = neighborhood_borders(u, edges)
        v_borders = neighborhood_borders(v, edges)

    if u_borders[0] < v_borders[0]:
        edges.append((v, u_borders[0]))
    elif u_borders[0] != v_borders[0]:
        edges.append((u, v_borders[0]))

    if u_borders[1] > v_borders[1]:
        edges.append((v, u_borders[1]))
    elif u_borders[1] != v_borders[1]:
        edges.append((u, v_borders[1]))

    return PaceGraph(a, b, edges)

if __name__ == "__main__":
    for i in range(10, 100):
        for _ in range(0, 100):
            #  Create test file
            rr2_instance_file = open("rr2_instance.gr", "w")
            print(random_rr2_graph(i, i + 3, P_EDGE * i).to_gr(), file=rr2_instance_file)
            rr2_instance_file.close()
            
            # Run solvers
            _ = run_command_with_limit("./solver", "rr2_instance.gr", "correct_output.sol", 2)
            _ = run_command_with_limit("./rr2_solver", "rr2_instance.gr", "rr2_output.sol", 2)
            
            # Parse output files for solution size
            correct_sol_file = open("correct_output.sol", "r")
            rr2_sol_file = open("rr2_output.sol", "r")
            time.sleep(1)
            while True:
                line = correct_sol_file.readline()
                if line.startswith("#Crossings"):
                    break
            correct_sol = int(line[12:])
            while True:
                line = rr2_sol_file.readline()
                if line.startswith("#Crossings"):
                    break
            rr2_sol = int(line[12:])
            correct_sol_file.close()
            rr2_sol_file.close()
            if rr2_sol != correct_sol:
                print("Found diverging solutions!")
                print(f"Correct: {correct_sol}")
                print(f"RR2: {rr2_sol}")
                exit(1)
            else:
                print(f"Crossings: {correct_sol}")




