cmake_minimum_required(VERSION 3.12)
project(pace2024)

set(CMAKE_CXX_STANDARD 17)
# Set the C and C++ compilers to GCC
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG -march=native")
add_compile_options(-mbmi2)

# Add PaceGraph source file     
add_library(PaceGraph src/pace_graph/pace_graph.cpp
        src/pace_graph/segment_tree.cpp
        src/pace_graph/segment_tree.hpp
        src/pace_graph/solver.hpp
        src/heuristic_solver/heuristic.hpp
        src/heuristic_solver/mean_position_heuristic.cpp
        src/heuristic_solver/mean_position_heuristic.hpp
        src/pace_graph/order.hpp
        src/pace_graph/relation.hpp
        )

# Add heuristic_solver executable
set(HEURISTIC_FILES
        src/exact/exact.cpp
        src/exact/exact.h
        src/heuristic_solver/local_search.cpp
        src/heuristic_solver/local_search.hpp
        src/heuristic_solver/genetic_algorithm.cpp
        src/heuristic_solver/genetic_algorithm.hpp
        src/heuristic_solver/mean_position_heuristic.cpp
        src/heuristic_solver/mean_position_heuristic.hpp
        src/lb/simple_lb.cpp
        src/lb/simple_lb.hpp
        src/heuristic_solver/heuristic_solver.hpp
        src/heuristic_solver/heuristic_solver.cpp
        src/heuristic_solver/heuristic.hpp
        )

add_executable(
        heuristic_solver
        src/heuristic_solver/main.cpp
        ${HEURISTIC_FILES}
        )

add_executable(
        lb_solver
        src/lb/main.cpp
        src/lb/simple_lb.cpp
        src/lb/simple_lb.hpp
        src/lb/lb_solver.hpp
        src/lb/lb_solver.cpp
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

# Test using doctest framework
option(ENABLE_TEST "Enable Test Build" OFF)

if (ENABLE_TEST)
        enable_testing()
        set(TEST_FILES
                tests/data_reductions.cpp
                tests/pace_graph.cpp
                )
        add_executable(tests ${HEURISTIC_FILES} ${TEST_FILES} tests/doctest.h)
        target_link_libraries(tests PaceGraph)
        add_test(NAME tests COMMAND tests)
endif()
