# PACE 2024 - Arcee - Student Submission

This is our submission to the [Pace Challenge 2024](https://pacechallenge.org/2024/).

> This year’s challenge is about the one-sided crossing minimization problem (OCM). This problem involves arranging the
> nodes of a bipartite graph on two layers (typically horizontal), with one of the layers fixed, aiming to minimize the
> number of edge crossings. OCM is one of the basic building block used for drawing hierarchical graphs. It is NP-hard,
> even for trees, but admits good heuristics, can be constant-factor approximated and solved in FPT time. For an
> extended
> overview, see Chapter 13.5 of the Handbook of Graph Drawing.
[[Pace](https://pacechallenge.org/2024/)]

## Contributors

- Kimon Boehmer (kimon.bohmer@universite-paris-saclay.fr)
- Lukas Lee George (l.george@tu-berlin.de)
- Fanny Hauser (f.hauser@tu-berlin.de)
- Jesse Palarus (j.palarus@tu-berlin.de)

## Requirements

- CMake 3.12 or higher
- a C++17 compiler (we used gcc/g++ 13.3.0)

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
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./heuristic_solver < path/to/your/gr.file
```

### Build Exact Solver and Parameterized solver

#### Requirements

- Google OR-Tools (Highly recommended)

Ensure you have installed Google OR-Tools on your system. Take a look at
the [official binaries](https://developers.google.com/optimization/install/cpp/binary_linux)
or [build and install it from source](https://developers.google.com/optimization/install/cpp/source_linux).
We have tested the code with Google OR-Tools version 9.10.

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_ILP_SOLVER=ON .. 
make
./feedback_edge_set_solver < path/to/your/gr.file
```

When everything is set up correctly, you should see somewhere the following output:

```sh
...
-- Check ILP Solver: ON
...
```

If your can not install Google OR-Tools on your system, you can use the following command to build the exact solver and
parameterized solver (but it is much slower). Make sure to remove the `build` directory before rebuilding the project
with ortools.

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release  .. 
make
./feedback_edge_set_solver < path/to/your/gr.file
```

### Submitting to Optil.io

Since Optil.io does not seems to build in Release mode, you can uncomment the following line in the `CMakeLists.txt`
file to sumbit to Optil.io:

```cmake
# Set the build type to Release (needed for optil.io)
#set(CMAKE_BUILD_TYPE Release)
```

### Rebuilding the project

Ensure you have removed the `build` directory before rebuilding the project. For example, when activate/deactivate the
ilp solver.

```sh
rm -rf build
``` 
