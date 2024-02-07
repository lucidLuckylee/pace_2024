cmake_minimum_required(VERSION 3.12)
project(pace2024)

set(CMAKE_CXX_STANDARD 17)


# Add PaceGraph source file
add_library(PaceGraph src/pace_graph/pace_graph.cpp)

# Add heuristic_solver executable
add_executable(heuristic_solver src/heuristic_solver/main.cpp
        src/exact/exact.cpp
        src/exact/exact.h
        src/heuristic_solver/local_search.cpp
        src/heuristic_solver/local_search.h
        src/heuristic_solver/genetic_algorithm.cpp
        src/heuristic_solver/genetic_algorithm.h)


# Link the executable with the PaceGraph library
target_link_libraries(heuristic_solver PaceGraph)

option(ENABLE_ILP_SOLVER "Enable ILP Solver Build and OR-Tools Download" OFF)

if (ENABLE_ILP_SOLVER)
    include(FetchContent)

    FetchContent_Declare(
            ortools
            GIT_REPOSITORY https://github.com/google/or-tools
            GIT_TAG stable
    )
    set(BUILD_DEPS ON CACHE BOOL "Build OR-Tools' dependencies")
    FetchContent_MakeAvailable(ortools)

    add_executable(ilp_solver src/ilp_solver/main.cpp)

    target_link_libraries(ilp_solver PaceGraph)
    target_link_libraries(ilp_solver ortools::ortools)
endif ()