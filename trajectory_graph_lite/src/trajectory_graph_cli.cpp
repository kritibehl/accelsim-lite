#include "trajectory_graph.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: trajectory-graph <trajectory.csv>\n";
        return 2;
    }

    try {
        const auto parsed = trajectory_graph::parse_trajectory_csv(argv[1]);
        const auto summary = parsed.graph.summarize(parsed.route);

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "{\n";
        std::cout << "  \"nodes\": " << summary.node_count << ",\n";
        std::cout << "  \"edges\": " << summary.edge_count << ",\n";
        std::cout << "  \"connected_components\": "
                  << summary.connected_components << ",\n";
        std::cout << "  \"disconnected_segments\": "
                  << summary.disconnected_segments << ",\n";
        std::cout << "  \"total_edge_distance\": "
                  << summary.total_edge_distance << ",\n";
        std::cout << "  \"average_edge_distance\": "
                  << summary.average_edge_distance << ",\n";
        std::cout << "  \"turn_transitions\": "
                  << summary.turn_transitions << "\n";
        std::cout << "}\n";
    } catch (const std::exception& error) {
        std::cerr << "trajectory analysis failed: "
                  << error.what() << "\n";
        return 1;
    }

    return 0;
}
