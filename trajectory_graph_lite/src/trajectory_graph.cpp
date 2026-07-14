#include "trajectory_graph.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace trajectory_graph {
namespace {

constexpr double kPi = 3.14159265358979323846;

double distance_between(const Point& a, const Point& b) {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

double turn_angle_degrees(
    const Point& a,
    const Point& b,
    const Point& c
) {
    const double v1x = b.x - a.x;
    const double v1y = b.y - a.y;
    const double v2x = c.x - b.x;
    const double v2y = c.y - b.y;

    const double mag1 = std::sqrt(v1x * v1x + v1y * v1y);
    const double mag2 = std::sqrt(v2x * v2x + v2y * v2y);

    if (mag1 == 0.0 || mag2 == 0.0) {
        return 0.0;
    }

    double cosine = (v1x * v2x + v1y * v2y) / (mag1 * mag2);
    cosine = std::clamp(cosine, -1.0, 1.0);

    return std::acos(cosine) * 180.0 / kPi;
}

}  // namespace

void TrajectoryGraph::add_point(const Point& point) {
    points_[point.id] = point;
    adjacency_.try_emplace(point.id);
}

void TrajectoryGraph::add_edge(std::uint64_t from, std::uint64_t to) {
    if (!has_point(from) || !has_point(to)) {
        throw std::invalid_argument("edge references unknown point");
    }

    const double distance = edge_distance(from, to);
    edges_.push_back({from, to, distance});

    adjacency_[from].push_back(to);
    adjacency_[to].push_back(from);
}

bool TrajectoryGraph::has_point(std::uint64_t id) const {
    return points_.find(id) != points_.end();
}

std::size_t TrajectoryGraph::node_count() const {
    return points_.size();
}

std::size_t TrajectoryGraph::edge_count() const {
    return edges_.size();
}

std::size_t TrajectoryGraph::connected_components() const {
    std::unordered_set<std::uint64_t> visited;
    std::size_t components = 0;

    for (const auto& [id, _] : points_) {
        if (visited.contains(id)) {
            continue;
        }

        ++components;
        std::queue<std::uint64_t> pending;
        pending.push(id);
        visited.insert(id);

        while (!pending.empty()) {
            const auto current = pending.front();
            pending.pop();

            const auto adjacency_it = adjacency_.find(current);
            if (adjacency_it == adjacency_.end()) {
                continue;
            }

            for (const auto neighbor : adjacency_it->second) {
                if (!visited.contains(neighbor)) {
                    visited.insert(neighbor);
                    pending.push(neighbor);
                }
            }
        }
    }

    return components;
}

std::vector<std::uint64_t> TrajectoryGraph::disconnected_nodes() const {
    std::vector<std::uint64_t> disconnected;

    for (const auto& [id, _] : points_) {
        const auto it = adjacency_.find(id);
        if (it == adjacency_.end() || it->second.empty()) {
            disconnected.push_back(id);
        }
    }

    std::sort(disconnected.begin(), disconnected.end());
    return disconnected;
}

double TrajectoryGraph::total_edge_distance() const {
    double total = 0.0;

    for (const auto& edge : edges_) {
        total += edge.distance;
    }

    return total;
}

double TrajectoryGraph::average_edge_distance() const {
    if (edges_.empty()) {
        return 0.0;
    }

    return total_edge_distance() / static_cast<double>(edges_.size());
}

std::size_t TrajectoryGraph::turn_transitions(
    const std::vector<std::uint64_t>& route,
    double minimum_turn_degrees
) const {
    if (route.size() < 3) {
        return 0;
    }

    std::size_t count = 0;

    for (std::size_t i = 1; i + 1 < route.size(); ++i) {
        const auto a = points_.find(route[i - 1]);
        const auto b = points_.find(route[i]);
        const auto c = points_.find(route[i + 1]);

        if (a == points_.end() ||
            b == points_.end() ||
            c == points_.end()) {
            throw std::invalid_argument("route references unknown point");
        }

        if (turn_angle_degrees(
                a->second,
                b->second,
                c->second
            ) >= minimum_turn_degrees) {
            ++count;
        }
    }

    return count;
}

GraphSummary TrajectoryGraph::summarize(
    const std::vector<std::uint64_t>& route
) const {
    const auto components = connected_components();

    return {
        node_count(),
        edge_count(),
        components,
        components > 0 ? components - 1 : 0,
        total_edge_distance(),
        average_edge_distance(),
        turn_transitions(route)
    };
}

const std::vector<Edge>& TrajectoryGraph::edges() const {
    return edges_;
}

double TrajectoryGraph::edge_distance(
    std::uint64_t from,
    std::uint64_t to
) const {
    return distance_between(points_.at(from), points_.at(to));
}

ParsedTrajectory parse_trajectory_csv(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("unable to open trajectory file: " + path);
    }

    ParsedTrajectory parsed;
    std::vector<std::pair<std::uint64_t, std::uint64_t>> pending_edges;

    std::string line;
    bool first_line = true;

    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        if (first_line) {
            first_line = false;
            if (line.rfind("id,", 0) == 0) {
                continue;
            }
        }

        std::stringstream stream(line);
        std::string id_text;
        std::string x_text;
        std::string y_text;
        std::string next_text;

        std::getline(stream, id_text, ',');
        std::getline(stream, x_text, ',');
        std::getline(stream, y_text, ',');
        std::getline(stream, next_text, ',');

        const auto id = static_cast<std::uint64_t>(std::stoull(id_text));
        const double x = std::stod(x_text);
        const double y = std::stod(y_text);

        parsed.graph.add_point({id, x, y});
        parsed.route.push_back(id);

        if (!next_text.empty() && next_text != "-1") {
            pending_edges.emplace_back(
                id,
                static_cast<std::uint64_t>(std::stoull(next_text))
            );
        }
    }

    for (const auto& [from, to] : pending_edges) {
        parsed.graph.add_edge(from, to);
    }

    return parsed;
}

}  // namespace trajectory_graph
