
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../src/data_reduction/data_reduction_rules.hpp"
#include "../src/pace_graph/pace_graph.hpp"
#include "doctest.h"

TEST_CASE("RR01 Simple") {
    std::string graph_gr =
        R"(p ocr 3 3 3
1 5
2 4
3 6)";
    std::istringstream gr_stream(graph_gr);
    PaceGraph graph = PaceGraph::from_gr(gr_stream);
    SUBCASE("Reading in the gr format") {
        CHECK(graph.size_free == 3);
        CHECK(graph.size_fixed == 3);
        // Edge {1, 5} has to exist
        CHECK(graph.neighbors_free[0][0] == 1);
    }

    graph.init_crossing_matrix_if_necessary();
    bool applied = rr1(graph);
    // 4 < 6
    CHECK(graph.crossing.lt(0, 2));
    // 5 < 6
    CHECK(graph.crossing.lt(1, 2));
    // 5 < 4
    CHECK(graph.crossing.lt(1, 0));

    CHECK(applied == true);

    applied = rr1(graph);
    // Previous partial_order should still exist
    // 4 < 6
    CHECK(graph.crossing.lt(0, 2));
    // 5 < 6
    CHECK(graph.crossing.lt(1, 2));
    // 5 < 4
    CHECK(graph.crossing.lt(1, 0));
    CHECK(applied == false);
}

TEST_CASE("RR02 Simple") {
    // N(4) = N(5) = N(7)
    std::string graph_gr =
        R"(p ocr 3 4 9
1 4
1 5
2 4
2 5
1 6
2 6
3 6
1 7
2 7
)";
    std::istringstream gr_stream(graph_gr);
    PaceGraph graph = PaceGraph::from_gr(gr_stream);

    graph.init_crossing_matrix_if_necessary();
    bool applied = rr2(graph);
    // 4 < 5
    CHECK(graph.crossing.lt(0, 1));
    CHECK(applied == true);

    applied = rr2(graph);
    // Previous partial_order should still exist
    // 4 < 5
    CHECK(graph.crossing.lt(0, 1));
    // 4 < 7
    CHECK(graph.crossing.lt(0, 3));
    // 5 < 7
    CHECK(graph.crossing.lt(1, 3));
    CHECK(applied == false);
}

TEST_CASE("RRLO1 Simple") {
    std::string graph_gr =
        R"(p ocr 3 5 10
1 4
1 5
2 4
2 5
1 6
2 6
3 6
1 7
2 7
3 8
)";
    std::istringstream gr_stream(graph_gr);
    PaceGraph graph = PaceGraph::from_gr(gr_stream);

    graph.init_crossing_matrix_if_necessary();
    rr1(graph);
    rr2(graph);
    // 4 < 5
    CHECK(graph.crossing.lt(0, 1));
    // 4 < 7
    CHECK(graph.crossing.lt(0, 3));
    // 5 < 7
    CHECK(graph.crossing.lt(1, 3));
    // 4 < 8
    CHECK(graph.crossing.lt(0, 4));
    // 5 < 8
    CHECK(graph.crossing.lt(1, 4));
    // 6 < 8
    CHECK(graph.crossing.lt(2, 4));
    // 7 < 8
    CHECK(graph.crossing.lt(3, 4));

    int old_size_free = graph.size_free;
    bool applied = rrlo1(graph);
    // Vertex 8 can be removed
    CHECK(applied == true);
    CHECK(old_size_free == graph.size_free + 1);
    // No more deletions possible
    applied = rrlo1(graph);
    CHECK(applied == false);
}

TEST_CASE("RRLO2 Simple") {
    std::string graph_gr =
        R"(p ocr 5 6 12
1 6
1 7
2 6
2 7
1 8
2 8
3 8
1 9
2 9
3 10
4 10
5 10
4 11
3 11
)";
    std::istringstream gr_stream(graph_gr);
    PaceGraph graph = PaceGraph::from_gr(gr_stream);

    graph.init_crossing_matrix_if_necessary();
    while (rr1(graph)) {
    };
    while (rr2(graph)) {
    };

    // 4 < 5
    CHECK(graph.crossing.lt(0, 1));
    // 4 < 7
    CHECK(graph.crossing.lt(0, 3));
    // 5 < 7
    CHECK(graph.crossing.lt(1, 3));
    // 4 < 8
    CHECK(graph.crossing.lt(0, 4));
    // 5 < 8
    CHECK(graph.crossing.lt(1, 4));
    // 6 < 8
    CHECK(graph.crossing.lt(2, 4));
    // 7 < 8
    CHECK(graph.crossing.lt(3, 4));

    bool applied = rrlo2(graph);
    CHECK(applied == true);
    // 11 < 10
    CHECK(graph.crossing.lt(5, 4));
}

TEST_CASE("Exhaustively Reduce") {
    std::string graph_gr = R"(p ocr 4 5 20
1 5
1 6
1 7
1 8
1 9
2 5
2 6
2 7
2 8
2 9
3 5
3 6
3 7
3 8
3 9
4 5
4 6
4 7
4 8
4 9)";

    std::istringstream gr_stream(graph_gr);
    PaceGraph graph = PaceGraph::from_gr(gr_stream);

    graph.init_crossing_matrix_if_necessary();
    apply_reduction_rules(graph);
    std::cout << graph.to_gr() << std::endl;
    CHECK(graph.to_gr().compare(
              R"(p ocr 4 0 0
)") == 0);
}
