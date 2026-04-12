#include <iostream>
#include "accelsim/core/simulator.hpp"
#include "accelsim/core/workload_parser.hpp"

using namespace accelsim;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./accelsim-lite <workload.csv>\n";
        return 1;
    }

    const std::string workload_path = argv[1];

    SimulatorConfig config;
    config.num_compute_units = 2;
    config.num_memory_ports = 1;
    config.queue_capacity = 4;

    auto workload = parse_workload_csv(workload_path);

    auto result = run_workload(workload, config);

    std::cout << "Throughput: " << result.throughput << "\n";
    std::cout << "Avg Latency: " << result.average_latency << "\n";
    std::cout << "Top Bottleneck: " << result.top_bottleneck << "\n";

    return 0;
}
