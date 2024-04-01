#include "../src/pace_graph/pace_graph.hpp"
#include "doctest.h"
#include <sstream>

TEST_CASE("Remove vertex") {
    std::string graph_gr =
        R"(p ocr 4 4 8
1 6
1 8
2 6
2 7
3 5
3 7
4 5
4 8
)";
    std::istringstream gr_stream(graph_gr);
    PaceGraph graph = PaceGraph::from_gr(gr_stream);
    graph.init_crossing_matrix_if_necessary();
    std::vector<std::tuple<int, int>> vertices_to_remove = {{1, 0}, {2, 1}};
    graph.remove_free_vertices(vertices_to_remove);
    SUBCASE("Check Graph") {
        CHECK(graph.size_free == 2);
        CHECK(graph.size_fixed == 4);

        CHECK(graph.neighbors_fixed[0] == std::vector<int>{1});
        CHECK(graph.neighbors_fixed[1] == std::vector<int>{});
        CHECK(graph.neighbors_fixed[2] == std::vector<int>{0});
        CHECK(graph.neighbors_fixed[3] == std::vector<int>{0, 1});

        CHECK(graph.neighbors_free[0] == std::vector<int>{2, 3});
        CHECK(graph.neighbors_free[1] == std::vector<int>{0, 3});

        std::stack<std::tuple<int, int>> removed_vertices =
            graph.removed_vertices;
        auto [v1, pos1] = removed_vertices.top();
        removed_vertices.pop();
        CHECK(v1 == 6);
        CHECK(pos1 == 0);

        auto [v2, pos2] = removed_vertices.top();
        removed_vertices.pop();
        CHECK(v2 == 7);
        CHECK(pos2 == 1);

        CHECK(removed_vertices.empty());

        CHECK(graph.free_real_names[0] == 5);
        CHECK(graph.free_real_names[1] == 8);
    }
    SUBCASE("Check Matrix") {
        CHECK(graph.crossing.matrix.size() == 2);
        CHECK(graph.crossing.matrix_diff.size() == 2);

        CHECK(graph.crossing.matrix[0][0] == 0);
        CHECK(graph.crossing.matrix[0][1] == 2);

        CHECK(graph.crossing.matrix[1][0] == 1);
        CHECK(graph.crossing.matrix[1][1] == 0);

        CHECK(graph.crossing.matrix_diff[0][0] == 0);
        CHECK(graph.crossing.matrix_diff[0][1] == 1);

        CHECK(graph.crossing.matrix_diff[1][0] == -1);
        CHECK(graph.crossing.matrix_diff[1][1] == 0);
    }
}
