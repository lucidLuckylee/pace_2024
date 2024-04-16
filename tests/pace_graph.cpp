#include "../src/pace_graph/pace_graph.hpp"
#include "doctest.h"
#include <cstdio>
#include <cstring>
#include <iostream>
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
    std::vector<DeleteInfo> vertices_to_remove = {DeleteInfo(1, 0, 3),
                                                  DeleteInfo(2, 1, 1)};
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

TEST_CASE("SPLIT_GRAPH_GR") {
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
    const int original_size_fixed = graph.size_fixed;

    std::tuple<std::vector<std::unique_ptr<PaceGraph>>, std::vector<int>> val =
        graph.splitGraphs();
    std::vector<std::unique_ptr<PaceGraph>> subgraphs =
        std::move(std::get<0>(val));
    std::vector<int> isolated_nodes = std::move(std::get<1>(val));

    CHECK(PaceGraph::split_graphs_to_gr(subgraphs, isolated_nodes,
                                        original_size_fixed)
              .compare(R"(p ocr 4 4 8
1 6
2 6
1 8
4 8
2 7
3 7
3 5
4 5
)") == 0);
}
