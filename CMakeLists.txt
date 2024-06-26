cmake_minimum_required(VERSION 3.12)
project(pace2024)


# Set the build type to Release (needed for optil.io)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG -march=native")

# Add PaceGraph source file     
add_library(PaceGraph src/pace_graph/pace_graph.cpp
        src/pace_graph/segment_tree.cpp
        src/pace_graph/segment_tree.hpp
        src/pace_graph/solver.hpp
        src/heuristic_solver/heuristic.hpp
        src/heuristic_solver/mean_position_heuristic.cpp
        src/heuristic_solver/mean_position_heuristic.hpp
        src/pace_graph/crossing_matrix.cpp
        src/pace_graph/crossing_matrix.hpp
        src/pace_graph/directed_graph.cpp
        src/pace_graph/directed_graph.hpp
        src/data_reduction/data_reduction_rules.cpp
        src/data_reduction/data_reduction_rules.hpp
        src/lb/simple_lb.cpp
        src/lb/simple_lb.hpp
)


# Add heuristic_solver executable
set(HEURISTIC_FILES
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
        src/heuristic_solver/greedy_insert_solver.cpp
        src/heuristic_solver/greedy_insert_solver.hpp
)

add_executable(
        feedback_edge_set_solver
        src/exact/feedback_edge_set_solver.cpp
        src/exact/feedback_edge_set_solver.hpp
        src/exact/feedback_edge_set_heuristic.cpp
        src/exact/feedback_edge_set_heuristic.cpp
        src/exact/main.cpp
        ${HEURISTIC_FILES}
)

add_executable(
        feedback_edge_set_heuristic
        src/exact/feedback_edge_set_heuristic_test.cpp
        src/exact/feedback_edge_set_solver.cpp
        src/exact/feedback_edge_set_solver.hpp
        src/exact/feedback_edge_set_heuristic.cpp
        src/exact/feedback_edge_set_heuristic.cpp
        ${HEURISTIC_FILES}
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

# Data Reduction stand-alone target
add_executable(data_reduction
        src/data_reduction/main.cpp
        src/heuristic_solver/greedy_insert_solver.cpp
        src/heuristic_solver/greedy_insert_solver.hpp
)
target_link_libraries(data_reduction PaceGraph)

# Crossing matrix stand-alone target
add_executable(
        crossing_matrix
        src/crossing_matrix/main.cpp
        src/heuristic_solver/greedy_insert_solver.cpp
        src/heuristic_solver/greedy_insert_solver.hpp
        ${HEURISTIC_FILES}
)
target_link_libraries(lb_solver PaceGraph)
# Link the executable with the PaceGraph library
target_link_libraries(heuristic_solver PaceGraph)
target_link_libraries(feedback_edge_set_solver PaceGraph)
target_link_libraries(crossing_matrix PaceGraph)
target_link_libraries(feedback_edge_set_heuristic PaceGraph)

option(ENABLE_ILP_SOLVER "Enable ILP Solver Build (Requires ortools)" OFF)
message(STATUS "Check ILP Solver: ${ENABLE_ILP_SOLVER}")

if (ENABLE_ILP_SOLVER)
    message(STATUS "ILP Solver Enabled")

    find_package(ortools CONFIG REQUIRED)
    add_compile_definitions(USE_ILP_TO_SOLVE_FES)

    add_library(
            fes_ilp_solver_lib
            src/exact/feedback_edge_set_ilp.cpp
            src/exact/feedback_edge_set_ilp.hpp
    )

    add_compile_definitions(USE_ILP_TO_SOLVE_FES)
    target_link_libraries(feedback_edge_set_solver fes_ilp_solver_lib)
    target_link_libraries(feedback_edge_set_solver ortools::ortools)

    target_link_libraries(feedback_edge_set_heuristic fes_ilp_solver_lib)
    target_link_libraries(feedback_edge_set_heuristic ortools::ortools)
endif ()


# Test using doctest framework
option(ENABLE_TEST "Enable Test Build" OFF)

if (ENABLE_TEST)
    enable_testing()
    set(TEST_FILES
            tests/data_reductions.cpp
            tests/pace_graph.cpp
            tests/directed_graph.cpp
            src/exact/feedback_edge_set_solver.cpp
            src/exact/feedback_edge_set_solver.hpp
            src/exact/feedback_edge_set_heuristic.cpp
            src/exact/feedback_edge_set_heuristic.cpp
            tests/fes_lb_test.cpp
    )
    add_executable(tests ${HEURISTIC_FILES} ${TEST_FILES} tests/doctest.h)
    target_link_libraries(tests PaceGraph)
    add_test(NAME tests COMMAND tests)
endif ()
