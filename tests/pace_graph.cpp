#include "../src/pace_graph/pace_graph.hpp"
#include "doctest.h"
#include <sstream>

TEST_CASE("Remove vertex") {
    std::string graph_gr =
        R"(p ocr 3 3 3
1 5
2 4
3 6)";
    std::istringstream gr_stream(graph_gr);
    PaceGraph graph = PaceGraph::from_gr(gr_stream);
    // Check that the graph is read in
    CHECK(graph.size_free == 3);
    CHECK(graph.size_fixed == 3);
    // Edge {1, 5} has to exist
    CHECK(graph.neighbors_free[0][0] == 1);

    graph.init_crossing_matrix_if_necessary();
    graph.remove_free_vertex(0);
    
    // c_{6, 5} == 1
    CHECK(graph.crossing_matrix[1][0] == 1);
    CHECK(graph.free_real_names[0] == 5);
    CHECK(graph.to_gr().compare(
R"(p ocr 3 2 2
1 5
3 6
)") == 0);
}
