#!/bin/bash

rm -f heuristic_solver.tgz
tar -czf heuristic_solver.tgz src CMakeLists.txt


rm -f feedback_edge_set_solver.tgz
tar -czf feedback_edge_set_solver.tgz src CMakeLists.txt

echo "submissions created."