#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace storage_datapath {

enum class Operation {
    Read,
    Write
};

struct Request {
    std::uint64_t id;
    Operation operation;
    std::uint64_t block;
    std::size_t size_bytes;
    bool cache_hit;
};

struct Config {
    std::size_t max_queue_depth = 8;
    double memory_bandwidth_bytes_per_us = 4096.0;
    double cache_hit_latency_us = 2.0;
    double cache_miss_latency_us = 18.0;
    double write_base_latency_us = 12.0;
    double queue_penalty_us = 1.5;
};

struct RequestResult {
    std::uint64_t id;
    double queue_delay_us;
    double service_latency_us;
    double total_latency_us;
    std::size_t queue_depth;
    bool cache_hit;
};

struct Summary {
    std::size_t total_requests;
    std::size_t read_requests;
    std::size_t write_requests;
    std::size_t cache_hits;
    std::size_t cache_misses;
    std::size_t max_observed_queue_depth;
    double throughput_requests_per_sec;
    double p50_latency_us;
    double p95_latency_us;
    double p99_latency_us;
    std::string bottleneck;
};

class Simulator {
public:
    explicit Simulator(Config config);

    std::vector<RequestResult> run(
        const std::vector<Request>& requests
    ) const;

    Summary summarize(
        const std::vector<Request>& requests,
        const std::vector<RequestResult>& results
    ) const;

private:
    double service_latency(const Request& request) const;
    std::string classify_bottleneck(
        const std::vector<Request>& requests,
        const std::vector<RequestResult>& results
    ) const;

    Config config_;
};

std::vector<Request> parse_workload_csv(const std::string& path);

}  // namespace storage_datapath
