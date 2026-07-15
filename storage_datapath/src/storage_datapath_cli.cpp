#include "storage_datapath.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: storage-datapath <workload.csv>\n";
        return 2;
    }

    try {
        const auto requests =
            storage_datapath::parse_workload_csv(argv[1]);

        storage_datapath::Simulator simulator({
            8,
            4096.0,
            2.0,
            18.0,
            12.0,
            1.5
        });

        const auto results = simulator.run(requests);
        const auto summary = simulator.summarize(requests, results);

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "{\n";
        std::cout << "  \"total_requests\": "
                  << summary.total_requests << ",\n";
        std::cout << "  \"read_requests\": "
                  << summary.read_requests << ",\n";
        std::cout << "  \"write_requests\": "
                  << summary.write_requests << ",\n";
        std::cout << "  \"cache_hits\": "
                  << summary.cache_hits << ",\n";
        std::cout << "  \"cache_misses\": "
                  << summary.cache_misses << ",\n";
        std::cout << "  \"max_queue_depth\": "
                  << summary.max_observed_queue_depth << ",\n";
        std::cout << "  \"throughput_requests_per_sec\": "
                  << summary.throughput_requests_per_sec << ",\n";
        std::cout << "  \"p50_latency_us\": "
                  << summary.p50_latency_us << ",\n";
        std::cout << "  \"p95_latency_us\": "
                  << summary.p95_latency_us << ",\n";
        std::cout << "  \"p99_latency_us\": "
                  << summary.p99_latency_us << ",\n";
        std::cout << "  \"bottleneck\": \""
                  << summary.bottleneck << "\"\n";
        std::cout << "}\n";
    } catch (const std::exception& error) {
        std::cerr << "storage datapath simulation failed: "
                  << error.what() << "\n";
        return 1;
    }

    return 0;
}
