#include <sstream>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../src/pace_graph/order.hpp"
#include "../src/pace_graph/pace_graph.hpp"
#include "doctest.h"

TEST_CASE("RR01") {
    std::string graph_gr =
        R"(p ocr 3 3 3
			1 5
			2 4
			3 6
		)";
    std::istringstream gr_stream(graph_gr);
    PaceGraph graph = PaceGraph::from_gr(gr_stream);
    SUBCASE("Reading in the gr format") {
        CHECK(graph.size_free == 3);
        CHECK(graph.size_fixed == 3);
        // Edge {1, 5} has to exist
        CHECK(graph.neighbors_free[0][0] == 1);
    }

    graph.init_crossing_matrix_if_necessary();
    Order order(graph.size_free);
    bool applied = order.rr1_rr2(graph);
    order.partial_order.print();
    // 4 < 6
    CHECK(order.partial_order.lt(0, 2));
    // 5 < 6
    CHECK(order.partial_order.lt(1, 2));
    // 5 < 4
    CHECK(order.partial_order.lt(1, 0));

    CHECK(applied == true);

    applied = order.rr1_rr2(graph);
    order.partial_order.print();
    // Previous partial_order should still exist
    // 4 < 6
    CHECK(order.partial_order.lt(0, 2));
    // 5 < 6
    CHECK(order.partial_order.lt(1, 2));
    // 5 < 4
    CHECK(order.partial_order.lt(1, 0));
    CHECK(applied == false);
}
