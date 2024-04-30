#include "../src/pace_graph/directed_graph.hpp"
#include "../src/pace_graph/pace_graph.hpp"
#include "doctest.h"
#include <sstream>

TEST_CASE("Transitive enclosure") {
    SUBCASE("Sparse Graph") {
        std::vector<std::vector<int>> adjNeig = {{2},    {6}, {}, {4},
                                                 {0, 7}, {3}, {}, {1}};

        DirectedGraph graph(adjNeig);
        graph.init_reachability_matrix_dag();

        CHECK(graph.reachabilityMatrix[0] ==
              std::bitset<MAX_MATRIX_SIZE>("00000100"));
        CHECK(graph.reachabilityMatrix[1] ==
              std::bitset<MAX_MATRIX_SIZE>("01000000"));
        CHECK(graph.reachabilityMatrix[2] ==
              std::bitset<MAX_MATRIX_SIZE>("00000000"));
        CHECK(graph.reachabilityMatrix[3] ==
              std::bitset<MAX_MATRIX_SIZE>("11010111"));
        CHECK(graph.reachabilityMatrix[4] ==
              std::bitset<MAX_MATRIX_SIZE>("11000111"));
        CHECK(graph.reachabilityMatrix[5] ==
              std::bitset<MAX_MATRIX_SIZE>("11011111"));
        CHECK(graph.reachabilityMatrix[6] ==
              std::bitset<MAX_MATRIX_SIZE>("00000000"));
        CHECK(graph.reachabilityMatrix[7] ==
              std::bitset<MAX_MATRIX_SIZE>("01000010"));
    }

    SUBCASE("Dense Graph") {
        std::vector<std::vector<int>> adjNeig = {
            {3}, {3, 5, 6}, {0, 4, 7}, {}, {1, 5}, {0}, {7}, {3}};

        DirectedGraph graph(adjNeig);
        graph.init_reachability_matrix_dag();

        CHECK(graph.reachabilityMatrix[0] ==
              std::bitset<MAX_MATRIX_SIZE>("00001000"));
        CHECK(graph.reachabilityMatrix[1] ==
              std::bitset<MAX_MATRIX_SIZE>("11101001"));
        CHECK(graph.reachabilityMatrix[2] ==
              std::bitset<MAX_MATRIX_SIZE>("11111011"));
        CHECK(graph.reachabilityMatrix[3] ==
              std::bitset<MAX_MATRIX_SIZE>("00000000"));
        CHECK(graph.reachabilityMatrix[4] ==
              std::bitset<MAX_MATRIX_SIZE>("11101011"));
        CHECK(graph.reachabilityMatrix[5] ==
              std::bitset<MAX_MATRIX_SIZE>("00001001"));
        CHECK(graph.reachabilityMatrix[6] ==
              std::bitset<MAX_MATRIX_SIZE>("10001000"));
        CHECK(graph.reachabilityMatrix[7] ==
              std::bitset<MAX_MATRIX_SIZE>("00001000"));
    }

    SUBCASE("Two Components Graph") {
        std::vector<std::vector<int>> adjNeig = {
            {1, 2}, {7}, {5, 6, 7, 10}, {0, 6}, {9, 12}, {7}, {7}, {},
            {9},    {},  {1},           {4, 8}, {9}};

        DirectedGraph graph(adjNeig);
        graph.init_reachability_matrix_dag();

        CHECK(graph.reachabilityMatrix[0] ==
              std::bitset<MAX_MATRIX_SIZE>("0010011100110"));
        CHECK(graph.reachabilityMatrix[1] ==
              std::bitset<MAX_MATRIX_SIZE>("0000010000000"));
        CHECK(graph.reachabilityMatrix[2] ==
              std::bitset<MAX_MATRIX_SIZE>("0010011100010"));
        CHECK(graph.reachabilityMatrix[3] ==
              std::bitset<MAX_MATRIX_SIZE>("0010011100111"));
        CHECK(graph.reachabilityMatrix[4] ==
              std::bitset<MAX_MATRIX_SIZE>("1001000000000"));
        CHECK(graph.reachabilityMatrix[5] ==
              std::bitset<MAX_MATRIX_SIZE>("0000010000000"));
        CHECK(graph.reachabilityMatrix[6] ==
              std::bitset<MAX_MATRIX_SIZE>("0000010000000"));
        CHECK(graph.reachabilityMatrix[7] ==
              std::bitset<MAX_MATRIX_SIZE>("0000000000000"));
        CHECK(graph.reachabilityMatrix[8] ==
              std::bitset<MAX_MATRIX_SIZE>("0001000000000"));
        CHECK(graph.reachabilityMatrix[9] ==
              std::bitset<MAX_MATRIX_SIZE>("0000000000000"));
        CHECK(graph.reachabilityMatrix[10] ==
              std::bitset<MAX_MATRIX_SIZE>("0000010000010"));
        CHECK(graph.reachabilityMatrix[11] ==
              std::bitset<MAX_MATRIX_SIZE>("1001100010000"));
        CHECK(graph.reachabilityMatrix[12] ==
              std::bitset<MAX_MATRIX_SIZE>("0001000000000"));
    }
}
