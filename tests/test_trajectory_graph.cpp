#include <gtest/gtest.h>

#include "trajectory_graph.h"

TEST(TrajectoryGraph, BuildsNodesAndEdges) {
    trajectory_graph::TrajectoryGraph graph;

    graph.add_point({1, 0.0, 0.0});
    graph.add_point({2, 3.0, 4.0});
    graph.add_edge(1, 2);

    EXPECT_EQ(graph.node_count(), 2u);
    EXPECT_EQ(graph.edge_count(), 1u);
    EXPECT_DOUBLE_EQ(graph.total_edge_distance(), 5.0);
}

TEST(TrajectoryGraph, ComputesConnectedComponents) {
    trajectory_graph::TrajectoryGraph graph;

    graph.add_point({1, 0.0, 0.0});
    graph.add_point({2, 1.0, 0.0});
    graph.add_point({3, 10.0, 10.0});

    graph.add_edge(1, 2);

    EXPECT_EQ(graph.connected_components(), 2u);

    const auto disconnected = graph.disconnected_nodes();
    ASSERT_EQ(disconnected.size(), 1u);
    EXPECT_EQ(disconnected.front(), 3u);
}

TEST(TrajectoryGraph, DetectsTurnTransitions) {
    trajectory_graph::TrajectoryGraph graph;

    graph.add_point({1, 0.0, 0.0});
    graph.add_point({2, 1.0, 0.0});
    graph.add_point({3, 1.0, 1.0});

    graph.add_edge(1, 2);
    graph.add_edge(2, 3);

    EXPECT_EQ(graph.turn_transitions({1, 2, 3}, 20.0), 1u);
}

TEST(TrajectoryGraph, ParsesDeterministicCSV) {
    const auto parsed = trajectory_graph::parse_trajectory_csv(
        std::string(PROJECT_SOURCE_DIR) +
        "/trajectory_graph_lite/data/connected_route.csv"
    );

    const auto summary = parsed.graph.summarize(parsed.route);

    EXPECT_EQ(summary.node_count, 5u);
    EXPECT_EQ(summary.edge_count, 4u);
    EXPECT_EQ(summary.connected_components, 1u);
    EXPECT_EQ(summary.disconnected_segments, 0u);
    EXPECT_EQ(summary.turn_transitions, 1u);
}
