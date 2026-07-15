#include "storage_datapath.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace storage_datapath {
namespace {

double percentile(std::vector<double> values, double p) {
    if (values.empty()) {
        return 0.0;
    }

    std::sort(values.begin(), values.end());

    const double index = p * static_cast<double>(values.size() - 1);
    const auto lower = static_cast<std::size_t>(std::floor(index));
    const auto upper = static_cast<std::size_t>(std::ceil(index));

    if (lower == upper) {
        return values[lower];
    }

    const double weight = index - static_cast<double>(lower);
    return values[lower] * (1.0 - weight) + values[upper] * weight;
}

}  // namespace

Simulator::Simulator(Config config)
    : config_(config) {}

std::vector<RequestResult> Simulator::run(
    const std::vector<Request>& requests
) const {
    std::vector<RequestResult> results;
    results.reserve(requests.size());

    double virtual_time_us = 0.0;
    double completion_time_us = 0.0;

    for (std::size_t i = 0; i < requests.size(); ++i) {
        const auto& request = requests[i];

        const std::size_t queue_depth = std::min(
            i + 1,
            config_.max_queue_depth
        );

        const double queue_delay =
            static_cast<double>(queue_depth - 1) *
            config_.queue_penalty_us;

        const double service = service_latency(request);
        const double total = queue_delay + service;

        virtual_time_us += 1.0;
        completion_time_us = std::max(completion_time_us, virtual_time_us) + total;

        results.push_back({
            request.id,
            queue_delay,
            service,
            total,
            queue_depth,
            request.cache_hit
        });
    }

    return results;
}

Summary Simulator::summarize(
    const std::vector<Request>& requests,
    const std::vector<RequestResult>& results
) const {
    std::size_t reads = 0;
    std::size_t writes = 0;
    std::size_t hits = 0;
    std::size_t misses = 0;
    std::size_t max_depth = 0;
    double total_runtime_us = 0.0;

    std::vector<double> latencies;
    latencies.reserve(results.size());

    for (const auto& request : requests) {
        if (request.operation == Operation::Read) {
            ++reads;
        } else {
            ++writes;
        }

        if (request.cache_hit) {
            ++hits;
        } else {
            ++misses;
        }
    }

    for (const auto& result : results) {
        latencies.push_back(result.total_latency_us);
        total_runtime_us += result.total_latency_us;
        max_depth = std::max(max_depth, result.queue_depth);
    }

    const double throughput =
        total_runtime_us == 0.0
            ? 0.0
            : static_cast<double>(results.size()) /
              (total_runtime_us / 1'000'000.0);

    return {
        requests.size(),
        reads,
        writes,
        hits,
        misses,
        max_depth,
        throughput,
        percentile(latencies, 0.50),
        percentile(latencies, 0.95),
        percentile(latencies, 0.99),
        classify_bottleneck(requests, results)
    };
}

double Simulator::service_latency(const Request& request) const {
    const double bandwidth_latency =
        static_cast<double>(request.size_bytes) /
        config_.memory_bandwidth_bytes_per_us;

    if (request.operation == Operation::Read) {
        const double base = request.cache_hit
            ? config_.cache_hit_latency_us
            : config_.cache_miss_latency_us;

        return base + bandwidth_latency;
    }

    return config_.write_base_latency_us + bandwidth_latency;
}

std::string Simulator::classify_bottleneck(
    const std::vector<Request>& requests,
    const std::vector<RequestResult>& results
) const {
    if (requests.empty()) {
        return "none";
    }

    std::size_t misses = 0;
    std::size_t queue_pressure = 0;
    std::size_t large_requests = 0;

    for (const auto& request : requests) {
        if (!request.cache_hit) {
            ++misses;
        }

        if (request.size_bytes >= 16384) {
            ++large_requests;
        }
    }

    for (const auto& result : results) {
        if (result.queue_depth >= config_.max_queue_depth) {
            ++queue_pressure;
        }
    }

    const double miss_rate =
        static_cast<double>(misses) /
        static_cast<double>(requests.size());

    const double queue_rate =
        static_cast<double>(queue_pressure) /
        static_cast<double>(results.size());

    const double large_request_rate =
        static_cast<double>(large_requests) /
        static_cast<double>(requests.size());

    if (queue_rate >= 0.40) {
        return "queue_depth_pressure";
    }

    if (miss_rate >= 0.50) {
        return "cache_miss_pressure";
    }

    if (large_request_rate >= 0.40) {
        return "memory_bandwidth_pressure";
    }

    return "balanced";
}

std::vector<Request> parse_workload_csv(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("unable to open workload: " + path);
    }

    std::vector<Request> requests;
    std::string line;
    bool first = true;

    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        if (first) {
            first = false;
            if (line.rfind("id,", 0) == 0) {
                continue;
            }
        }

        std::stringstream stream(line);
        std::string id_text;
        std::string op_text;
        std::string block_text;
        std::string size_text;
        std::string hit_text;

        std::getline(stream, id_text, ',');
        std::getline(stream, op_text, ',');
        std::getline(stream, block_text, ',');
        std::getline(stream, size_text, ',');
        std::getline(stream, hit_text, ',');

        requests.push_back({
            static_cast<std::uint64_t>(std::stoull(id_text)),
            op_text == "read" ? Operation::Read : Operation::Write,
            static_cast<std::uint64_t>(std::stoull(block_text)),
            static_cast<std::size_t>(std::stoull(size_text)),
            hit_text == "true"
        });
    }

    return requests;
}

}  // namespace storage_datapath
