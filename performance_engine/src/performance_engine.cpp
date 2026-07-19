#include "performance_engine.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace performance_engine {

namespace {

const char* access_pattern_name(AccessPattern pattern) {
    return pattern == AccessPattern::CacheLocal
        ? "cache_local"
        : "cache_unfriendly";
}

}  // namespace

double percentile(std::vector<double> values, double fraction) {
    if (values.empty()) {
        return 0.0;
    }

    std::sort(values.begin(), values.end());

    const double position =
        fraction * static_cast<double>(values.size() - 1);

    const auto lower =
        static_cast<std::size_t>(std::floor(position));

    const auto upper =
        static_cast<std::size_t>(std::ceil(position));

    if (lower == upper) {
        return values[lower];
    }

    const double weight =
        position - static_cast<double>(lower);

    return values[lower] * (1.0 - weight)
        + values[upper] * weight;
}

PerformanceEngine::PerformanceEngine(BenchmarkConfig config)
    : config_(config),
      data_(config.workload_size) {
    if (config_.worker_count == 0) {
        throw std::invalid_argument(
            "worker_count must be greater than zero"
        );
    }

    if (config_.chunk_size == 0) {
        throw std::invalid_argument(
            "chunk_size must be greater than zero"
        );
    }

    for (std::size_t i = 0; i < data_.size(); ++i) {
        data_[i] =
            static_cast<std::uint64_t>((i * 2654435761ULL) ^ (i >> 3));
    }
}

std::vector<WorkItem> PerformanceEngine::build_work_items() const {
    std::vector<WorkItem> items;

    std::uint64_t id = 0;

    for (
        std::size_t begin = 0;
        begin < config_.workload_size;
        begin += config_.chunk_size
    ) {
        const std::size_t end =
            std::min(begin + config_.chunk_size, config_.workload_size);

        items.push_back({id++, begin, end});
    }

    return items;
}

std::uint64_t PerformanceEngine::process_work(
    const WorkItem& work,
    WorkerMetrics& metrics
) const {
    std::uint64_t local_checksum = 0;

    std::vector<std::uint64_t> temporary;
    temporary.reserve(work.end - work.begin);
    ++metrics.simulated_allocations;

    if (config_.access_pattern == AccessPattern::CacheLocal) {
        for (std::size_t i = work.begin; i < work.end; ++i) {
            const std::uint64_t value =
                data_[i] ^ static_cast<std::uint64_t>(i * 17);

            temporary.push_back(value);
            local_checksum += value;
        }
    } else {
        const std::size_t span = work.end - work.begin;

        if (span == 0) {
            return 0;
        }

        constexpr std::size_t stride = 67;

        for (std::size_t offset = 0; offset < span; ++offset) {
            const std::size_t index =
                work.begin + ((offset * stride) % span);

            const std::uint64_t value =
                data_[index]
                ^ static_cast<std::uint64_t>(index * 17);

            temporary.push_back(value);
            local_checksum += value;
        }
    }

    std::sort(temporary.begin(), temporary.end());

    for (std::size_t i = 1; i < temporary.size(); ++i) {
        local_checksum ^=
            temporary[i] - temporary[i - 1];
    }

    ++metrics.tasks_completed;
    return local_checksum;
}

BenchmarkResult PerformanceEngine::run_single_threaded() {
    const auto work_items = build_work_items();

    WorkerMetrics metrics;
    std::vector<double> latencies;
    latencies.reserve(work_items.size());

    std::uint64_t checksum = 0;

    const auto benchmark_start =
        std::chrono::steady_clock::now();

    for (const auto& item : work_items) {
        const auto task_start =
            std::chrono::steady_clock::now();

        checksum ^= process_work(item, metrics);

        const auto task_end =
            std::chrono::steady_clock::now();

        latencies.push_back(
            std::chrono::duration<double, std::micro>(
                task_end - task_start
            ).count()
        );
    }

    const auto benchmark_end =
        std::chrono::steady_clock::now();

    const double elapsed_ms =
        std::chrono::duration<double, std::milli>(
            benchmark_end - benchmark_start
        ).count();

    return build_result(
        "single_threaded",
        latencies,
        metrics,
        checksum,
        elapsed_ms,
        work_items.size()
    );
}

BenchmarkResult PerformanceEngine::run_multithreaded() {
    const auto work_items = build_work_items();

    std::deque<QueuedTask> queue;
    std::mutex queue_mutex;
    std::condition_variable queue_not_empty;
    std::condition_variable queue_not_full;

    bool producer_finished = false;

    std::mutex metrics_mutex;
    WorkerMetrics aggregate_metrics;
    std::vector<double> aggregate_latencies;
    aggregate_latencies.reserve(work_items.size());

    std::atomic<std::uint64_t> checksum{0};

    auto worker = [&]() {
        WorkerMetrics local_metrics;
        std::vector<double> local_latencies;

        while (true) {
            QueuedTask task;

            const auto lock_attempt =
                std::chrono::steady_clock::now();

            std::unique_lock<std::mutex> lock(
                queue_mutex,
                std::defer_lock
            );

            if (!lock.try_lock()) {
                ++local_metrics.contended_lock_acquisitions;
                lock.lock();
            }

            ++local_metrics.lock_acquisitions;

            const auto lock_acquired =
                std::chrono::steady_clock::now();

            local_metrics.lock_wait_ns +=
                static_cast<std::uint64_t>(
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        lock_acquired - lock_attempt
                    ).count()
                );

            queue_not_empty.wait(
                lock,
                [&]() {
                    return !queue.empty() || producer_finished;
                }
            );

            if (queue.empty() && producer_finished) {
                break;
            }

            task = queue.front();
            queue.pop_front();

            queue_not_full.notify_one();
            lock.unlock();

            const auto execution_start =
                std::chrono::steady_clock::now();

            local_metrics.queue_wait_ns +=
                static_cast<std::uint64_t>(
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        execution_start - task.queued_at
                    ).count()
                );

            const std::uint64_t task_checksum =
                process_work(task.work, local_metrics);

            checksum.fetch_xor(
                task_checksum,
                std::memory_order_relaxed
            );

            const auto execution_end =
                std::chrono::steady_clock::now();

            local_latencies.push_back(
                std::chrono::duration<double, std::micro>(
                    execution_end - task.queued_at
                ).count()
            );
        }

        std::lock_guard<std::mutex> guard(metrics_mutex);

        aggregate_metrics.tasks_completed +=
            local_metrics.tasks_completed;

        aggregate_metrics.simulated_allocations +=
            local_metrics.simulated_allocations;

        aggregate_metrics.lock_acquisitions +=
            local_metrics.lock_acquisitions;

        aggregate_metrics.contended_lock_acquisitions +=
            local_metrics.contended_lock_acquisitions;

        aggregate_metrics.lock_wait_ns +=
            local_metrics.lock_wait_ns;

        aggregate_metrics.queue_wait_ns +=
            local_metrics.queue_wait_ns;

        aggregate_latencies.insert(
            aggregate_latencies.end(),
            local_latencies.begin(),
            local_latencies.end()
        );
    };

    const auto benchmark_start =
        std::chrono::steady_clock::now();

    std::vector<std::thread> workers;
    workers.reserve(config_.worker_count);

    for (std::size_t i = 0; i < config_.worker_count; ++i) {
        workers.emplace_back(worker);
    }

    for (const auto& item : work_items) {
        std::unique_lock<std::mutex> lock(queue_mutex);

        queue_not_full.wait(
            lock,
            [&]() {
                return queue.size() < config_.queue_capacity;
            }
        );

        queue.push_back({
            item,
            std::chrono::steady_clock::now()
        });

        queue_not_empty.notify_one();
    }

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        producer_finished = true;
    }

    queue_not_empty.notify_all();

    for (auto& thread : workers) {
        thread.join();
    }

    const auto benchmark_end =
        std::chrono::steady_clock::now();

    const double elapsed_ms =
        std::chrono::duration<double, std::milli>(
            benchmark_end - benchmark_start
        ).count();

    return build_result(
        "multithreaded",
        aggregate_latencies,
        aggregate_metrics,
        checksum.load(std::memory_order_relaxed),
        elapsed_ms,
        work_items.size()
    );
}

BenchmarkResult PerformanceEngine::build_result(
    const std::string& mode,
    const std::vector<double>& task_latencies_us,
    const WorkerMetrics& metrics,
    std::uint64_t checksum,
    double elapsed_ms,
    std::size_t task_count
) const {
    const double throughput =
        elapsed_ms == 0.0
            ? 0.0
            : static_cast<double>(config_.workload_size)
                / (elapsed_ms / 1000.0);

    const double contention_rate =
        metrics.lock_acquisitions == 0
            ? 0.0
            : static_cast<double>(
                metrics.contended_lock_acquisitions
              ) / static_cast<double>(metrics.lock_acquisitions);

    return {
        mode,
        access_pattern_name(config_.access_pattern),
        config_.worker_count,
        config_.workload_size,
        task_count,
        elapsed_ms,
        throughput,
        percentile(task_latencies_us, 0.50),
        percentile(task_latencies_us, 0.95),
        percentile(task_latencies_us, 0.99),
        metrics.simulated_allocations,
        metrics.lock_acquisitions,
        metrics.contended_lock_acquisitions,
        contention_rate,
        static_cast<double>(metrics.lock_wait_ns) / 1'000'000.0,
        static_cast<double>(metrics.queue_wait_ns) / 1'000'000.0,
        classify_bottleneck(metrics, elapsed_ms),
        checksum
    };
}

std::string PerformanceEngine::classify_bottleneck(
    const WorkerMetrics& metrics,
    double elapsed_ms
) const {
    const double contention_rate =
        metrics.lock_acquisitions == 0
            ? 0.0
            : static_cast<double>(
                metrics.contended_lock_acquisitions
              ) / static_cast<double>(metrics.lock_acquisitions);

    const double lock_wait_ms =
        static_cast<double>(metrics.lock_wait_ns) / 1'000'000.0;

    const double queue_wait_ms =
        static_cast<double>(metrics.queue_wait_ns) / 1'000'000.0;

    if (
        config_.access_pattern == AccessPattern::CacheUnfriendly
        && elapsed_ms > 0.0
    ) {
        return "cache_locality_pressure";
    }

    if (contention_rate >= 0.15 || lock_wait_ms > elapsed_ms * 0.20) {
        return "lock_contention";
    }

    if (queue_wait_ms > elapsed_ms * 0.50) {
        return "queue_wait_pressure";
    }

    if (
        metrics.simulated_allocations
        > config_.workload_size / config_.chunk_size
    ) {
        return "allocation_pressure";
    }

    return "balanced";
}

}  // namespace performance_engine
