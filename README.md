# PACE 2024

## Project Structure
We opted to keep .hpp files in the same folder as their .cpp counterpart.
The rest of the layout is based on [The Pithfork Layout](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs).

## Build
```sh
cd build
cmake ..
make
# Substitute heuristic_solver with the name of the target you want to run
./heuristic_solver

# Useful as well
gdb heuristic_solver
valgrind heuristic_solver
```
