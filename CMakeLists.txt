cmake_minimum_required(VERSION 3.12)
project(pace2024)

set(CMAKE_CXX_STANDARD 17)

# Enable debugging info
set(CMAKE_BUILD_TYPE Debug)

# Add PaceGraph source file
add_library(PaceGraph src/pace_graph/pace_graph.cpp)

# Add heuristic_solver executable
add_executable(heuristic_solver src/heuristic_solver/main.cpp)

# Link the executable with the PaceGraph library
target_link_libraries(heuristic_solver PaceGraph)
