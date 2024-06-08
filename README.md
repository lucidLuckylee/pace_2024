# PACE 2024 Student Submission

## Set Timelimit for the heuristic solver

Maybe it is useful to set a timelimit for the heuristic solver. You can do by editing the following line in
the `src/heuristic_solver/heuristic_solver.hpp` file.

```cpp
 public:
    explicit HeuristicSolver(std::chrono::milliseconds limit =
                                 std::chrono::milliseconds(1000 * 60 * 5 -
                                                           1000 * 15))
        : SolutionSolver(limit) {}                      
```

By default we set a timelimit of 4 miniutes and 45 seconds. You can change the value of `1000 * 60 * 5 - 1000 * 15` to
any value you want.

## Build

### Build Heuristic Solver

```sh
cd build
cmake ..
make
./heuristic_solver < path/to/your/gr.file
```

### Build Exact Solver and Parameterized solver

#### Requirements

- Google OR-Tools

Ensure you have installed Google OR-Tools on your system.
We have tested the code with Google OR-Tools version 9.10.

```sh
cd build
cmake -DENABLE_ILP_SOLVER=ON .. 
make
./feedback_edge_set_solver < path/to/your/gr.file
```

If your can not install Google OR-Tools on your system, you can use the following command to build the exact solver and
parameterized solver (but it is much slower).

```sh
cd build
cmake .. 
make
./feedback_edge_set_solver < path/to/your/gr.file
```