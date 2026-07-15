#include <gtest/gtest.h>

#include "storage_datapath.h"

TEST(StorageDatapath, TracksQueueDepth) {
    storage_datapath::Simulator simulator({
        4,
        4096.0,
        2.0,
        18.0,
        12.0,
        1.5
    });

    const std::vector<storage_datapath::Request> requests = {
        {1, storage_datapath::Operation::Read, 1, 4096, true},
        {2, storage_datapath::Operation::Read, 2, 4096, true},
        {3, storage_datapath::Operation::Write, 3, 4096, false},
        {4, storage_datapath::Operation::Read, 4, 4096, false},
        {5, storage_datapath::Operation::Write, 5, 4096, false}
    };

    const auto results = simulator.run(requests);
    const auto summary = simulator.summarize(requests, results);

    EXPECT_EQ(summary.total_requests, 5u);
    EXPECT_EQ(summary.max_observed_queue_depth, 4u);
}

TEST(StorageDatapath, CacheHitIsFasterThanMiss) {
    storage_datapath::Simulator simulator({});

    const std::vector<storage_datapath::Request> hit = {
        {1, storage_datapath::Operation::Read, 1, 4096, true}
    };

    const std::vector<storage_datapath::Request> miss = {
        {1, storage_datapath::Operation::Read, 1, 4096, false}
    };

    const auto hit_result = simulator.run(hit);
    const auto miss_result = simulator.run(miss);

    EXPECT_LT(
        hit_result.front().service_latency_us,
        miss_result.front().service_latency_us
    );
}

TEST(StorageDatapath, ComputesLatencyPercentiles) {
    storage_datapath::Simulator simulator({});

    const std::vector<storage_datapath::Request> requests = {
        {1, storage_datapath::Operation::Read, 1, 4096, true},
        {2, storage_datapath::Operation::Read, 2, 4096, false},
        {3, storage_datapath::Operation::Write, 3, 8192, false},
        {4, storage_datapath::Operation::Read, 4, 16384, false}
    };

    const auto results = simulator.run(requests);
    const auto summary = simulator.summarize(requests, results);

    EXPECT_LE(summary.p50_latency_us, summary.p95_latency_us);
    EXPECT_LE(summary.p95_latency_us, summary.p99_latency_us);
    EXPECT_GT(summary.throughput_requests_per_sec, 0.0);
}

TEST(StorageDatapath, ClassifiesCachePressure) {
    storage_datapath::Simulator simulator({
        32,
        4096.0,
        2.0,
        18.0,
        12.0,
        0.1
    });

    const std::vector<storage_datapath::Request> requests = {
        {1, storage_datapath::Operation::Read, 1, 4096, false},
        {2, storage_datapath::Operation::Read, 2, 4096, false},
        {3, storage_datapath::Operation::Read, 3, 4096, false},
        {4, storage_datapath::Operation::Read, 4, 4096, true}
    };

    const auto results = simulator.run(requests);
    const auto summary = simulator.summarize(requests, results);

    EXPECT_EQ(summary.bottleneck, "cache_miss_pressure");
}
