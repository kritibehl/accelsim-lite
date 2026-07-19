#include "performance_engine.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::size_t parse_size(
    const char* value,
    const char* name
) {
    try {
        return static_cast<std::size_t>(
            std::stoull(value)
        );
    } catch (...) {
        throw std::invalid_argument(
            std::string("invalid ") + name
        );
    }
}

void print_result(
    const performance_engine::BenchmarkResult& result
) {
    std::cout << std::fixed << std::setprecision(4);

    std::cout
        << result.mode << ","
        << result.access_pattern << ","
        << result.worker_count << ","
        << result.workload_size << ","
        << result.tasks_processed << ","
        << result.elapsed_ms << ","
        << result.throughput_items_per_second << ","
        << result.p50_task_latency_us << ","
        << result.p95_task_latency_us << ","
        << result.p99_task_latency_us << ","
        << result.simulated_allocations << ","
        << result.lock_acquisitions << ","
        << result.contended_lock_acquisitions << ","
        << result.contention_rate << ","
        << result.total_lock_wait_ms << ","
        << result.total_queue_wait_ms << ","
        << result.bottleneck << ","
        << result.checksum
        << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr
            << "usage: performance-benchmark "
            << "<mode: single|multi> "
            << "<workers> "
            << "<workload_size> "
            << "<pattern: local|unfriendly>\n";

        return 2;
    }

    try {
        const std::string mode = argv[1];

        const std::size_t workers =
            parse_size(argv[2], "worker count");

        const std::size_t workload =
            parse_size(argv[3], "workload size");

        const std::string pattern_text = argv[4];

        const auto pattern =
            pattern_text == "local"
                ? performance_engine::AccessPattern::CacheLocal
                : performance_engine::AccessPattern::CacheUnfriendly;

        performance_engine::PerformanceEngine engine({
            workers,
            workload,
            256,
            1024,
            pattern
        });

        std::cout
            << "mode,access_pattern,worker_count,workload_size,"
            << "tasks_processed,elapsed_ms,throughput_items_per_second,"
            << "p50_task_latency_us,p95_task_latency_us,"
            << "p99_task_latency_us,simulated_allocations,"
            << "lock_acquisitions,contended_lock_acquisitions,"
            << "contention_rate,total_lock_wait_ms,"
            << "total_queue_wait_ms,bottleneck,checksum\n";

        if (mode == "single") {
            print_result(engine.run_single_threaded());
        } else if (mode == "multi") {
            print_result(engine.run_multithreaded());
        } else {
            throw std::invalid_argument(
                "mode must be single or multi"
            );
        }
    } catch (const std::exception& error) {
        std::cerr
            << "performance benchmark failed: "
            << error.what()
            << "\n";

        return 1;
    }

    return 0;
}
