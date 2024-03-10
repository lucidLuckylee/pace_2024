cmake_minimum_required(VERSION 3.12)
project(pace2024)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG -march=native")

# Add PaceGraph source file
add_library(PaceGraph src/pace_graph/pace_graph.cpp
        src/pace_graph/segment_tree.cpp
        src/pace_graph/segment_tree.hpp)

# Add heuristic_solver executable
add_executable(
        heuristic_solver
        src/heuristic_solver/main.cpp
        src/exact/exact.cpp
        src/exact/exact.h
        src/heuristic_solver/local_search.cpp
        src/heuristic_solver/local_search.hpp
        src/heuristic_solver/genetic_algorithm.cpp
        src/heuristic_solver/genetic_algorithm.hpp
        src/heuristic_solver/cheap_heuristics.cpp
        src/heuristic_solver/cheap_heuristics.hpp
        src/pace_graph/solver.cpp
        src/lb/simple_lb.cpp
        src/lb/simple_lb.hpp
)

add_executable(
        lb_solver
        src/lb/main.cpp
        src/lb/simple_lb.cpp
        src/lb/simple_lb.hpp
)

target_link_libraries(lb_solver PaceGraph)
# Link the executable with the PaceGraph library
target_link_libraries(heuristic_solver PaceGraph)

option(ENABLE_ILP_SOLVER "Enable ILP Solver Build (Requires ortools)" OFF)

if (ENABLE_ILP_SOLVER)
    find_package(ortools CONFIG REQUIRED)

add_executable(ilp_solver src/ilp_solver/main.cpp
        src/ilp_solver/ilp_solver.cpp
        src/ilp_solver/ilp_solver.hpp)

    target_link_libraries(ilp_solver PaceGraph)
    target_link_libraries(ilp_solver ortools::ortools)
endif ()
