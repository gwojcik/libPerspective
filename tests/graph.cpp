#include <catch2/catch.hpp>
#include "../Graph.h"

TEST_CASE ( "Graph" ) {
    using namespace Catch::literals;
    SECTION ( "create_from_structure" ) {
        GraphBase graph;
        RawGraph data = {
            .nodes = {},
            .edges = {},
            .root = "test",
            .version = nullptr,
            .visualizations = {},
        };
        REQUIRE_THROWS_WITH(graph.create_from_structure(data), "bad structure - Graph");
    }
}
