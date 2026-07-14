#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace trajectory_graph {

struct Point {
    std::uint64_t id;
    double x;
    double y;
};

struct Edge {
    std::uint64_t from;
    std::uint64_t to;
    double distance;
};

struct GraphSummary {
    std::size_t node_count;
    std::size_t edge_count;
    std::size_t connected_components;
    std::size_t disconnected_segments;
    double total_edge_distance;
    double average_edge_distance;
    std::size_t turn_transitions;
};

class TrajectoryGraph {
public:
    void add_point(const Point& point);
    void add_edge(std::uint64_t from, std::uint64_t to);

    bool has_point(std::uint64_t id) const;
    std::size_t node_count() const;
    std::size_t edge_count() const;

    std::size_t connected_components() const;
    std::vector<std::uint64_t> disconnected_nodes() const;

    double total_edge_distance() const;
    double average_edge_distance() const;

    std::size_t turn_transitions(
        const std::vector<std::uint64_t>& route,
        double minimum_turn_degrees = 20.0
    ) const;

    GraphSummary summarize(
        const std::vector<std::uint64_t>& route = {}
    ) const;

    const std::vector<Edge>& edges() const;

private:
    double edge_distance(std::uint64_t from, std::uint64_t to) const;

    std::unordered_map<std::uint64_t, Point> points_;
    std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> adjacency_;
    std::vector<Edge> edges_;
};

struct ParsedTrajectory {
    TrajectoryGraph graph;
    std::vector<std::uint64_t> route;
};

ParsedTrajectory parse_trajectory_csv(const std::string& path);

}  // namespace trajectory_graph
