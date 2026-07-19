#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace performance_engine {

enum class AccessPattern {
    CacheLocal,
    CacheUnfriendly
};

struct WorkItem {
    std::uint64_t id = 0;
    std::size_t begin = 0;
    std::size_t end = 0;
};

struct BenchmarkConfig {
    std::size_t worker_count = 1;
    std::size_t workload_size = 10000;
    std::size_t chunk_size = 256;
    std::size_t queue_capacity = 1024;
    AccessPattern access_pattern = AccessPattern::CacheLocal;
};

struct WorkerMetrics {
    std::uint64_t tasks_completed = 0;
    std::uint64_t simulated_allocations = 0;
    std::uint64_t lock_acquisitions = 0;
    std::uint64_t contended_lock_acquisitions = 0;
    std::uint64_t lock_wait_ns = 0;
    std::uint64_t queue_wait_ns = 0;
};

struct BenchmarkResult {
    std::string mode;
    std::string access_pattern;
    std::size_t worker_count = 0;
    std::size_t workload_size = 0;
    std::size_t tasks_processed = 0;

    double elapsed_ms = 0.0;
    double throughput_items_per_second = 0.0;

    double p50_task_latency_us = 0.0;
    double p95_task_latency_us = 0.0;
    double p99_task_latency_us = 0.0;

    std::uint64_t simulated_allocations = 0;
    std::uint64_t lock_acquisitions = 0;
    std::uint64_t contended_lock_acquisitions = 0;
    double contention_rate = 0.0;
    double total_lock_wait_ms = 0.0;
    double total_queue_wait_ms = 0.0;

    std::string bottleneck;
    std::uint64_t checksum = 0;
};

class PerformanceEngine {
public:
    explicit PerformanceEngine(BenchmarkConfig config);

    BenchmarkResult run_single_threaded();
    BenchmarkResult run_multithreaded();

private:
    struct QueuedTask {
        WorkItem work;
        std::chrono::steady_clock::time_point queued_at;
    };

    std::uint64_t process_work(
        const WorkItem& work,
        WorkerMetrics& metrics
    ) const;

    std::vector<WorkItem> build_work_items() const;

    BenchmarkResult build_result(
        const std::string& mode,
        const std::vector<double>& task_latencies_us,
        const WorkerMetrics& metrics,
        std::uint64_t checksum,
        double elapsed_ms,
        std::size_t task_count
    ) const;

    std::string classify_bottleneck(
        const WorkerMetrics& metrics,
        double elapsed_ms
    ) const;

    BenchmarkConfig config_;
    std::vector<std::uint64_t> data_;
};

double percentile(std::vector<double> values, double fraction);

}  // namespace performance_engine
