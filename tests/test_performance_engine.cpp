#include <gtest/gtest.h>

#include "performance_engine.h"

TEST(PerformanceEngine, SingleThreadProcessesAllWork) {
    performance_engine::PerformanceEngine engine({
        1,
        4096,
        256,
        64,
        performance_engine::AccessPattern::CacheLocal
    });

    const auto result = engine.run_single_threaded();

    EXPECT_EQ(result.workload_size, 4096u);
    EXPECT_EQ(result.tasks_processed, 16u);
    EXPECT_EQ(result.simulated_allocations, 16u);
    EXPECT_GT(result.throughput_items_per_second, 0.0);
    EXPECT_LE(result.p50_task_latency_us, result.p95_task_latency_us);
    EXPECT_LE(result.p95_task_latency_us, result.p99_task_latency_us);
}

TEST(PerformanceEngine, MultithreadedProcessesAllWork) {
    performance_engine::PerformanceEngine engine({
        4,
        8192,
        256,
        64,
        performance_engine::AccessPattern::CacheLocal
    });

    const auto result = engine.run_multithreaded();

    EXPECT_EQ(result.worker_count, 4u);
    EXPECT_EQ(result.workload_size, 8192u);
    EXPECT_EQ(result.tasks_processed, 32u);
    EXPECT_EQ(result.simulated_allocations, 32u);
    EXPECT_GT(result.lock_acquisitions, 0u);
    EXPECT_GT(result.throughput_items_per_second, 0.0);
}

TEST(PerformanceEngine, ResultsRemainDeterministic) {
    performance_engine::PerformanceEngine single_engine({
        1,
        4096,
        256,
        64,
        performance_engine::AccessPattern::CacheLocal
    });

    performance_engine::PerformanceEngine multi_engine({
        4,
        4096,
        256,
        64,
        performance_engine::AccessPattern::CacheLocal
    });

    const auto single =
        single_engine.run_single_threaded();

    const auto multi =
        multi_engine.run_multithreaded();

    EXPECT_EQ(single.checksum, multi.checksum);
}

TEST(PerformanceEngine, CachePatternsProduceSameChecksum) {
    performance_engine::PerformanceEngine local_engine({
        1,
        4096,
        256,
        64,
        performance_engine::AccessPattern::CacheLocal
    });

    performance_engine::PerformanceEngine unfriendly_engine({
        1,
        4096,
        256,
        64,
        performance_engine::AccessPattern::CacheUnfriendly
    });

    const auto local =
        local_engine.run_single_threaded();

    const auto unfriendly =
        unfriendly_engine.run_single_threaded();

    EXPECT_EQ(local.checksum, unfriendly.checksum);
}

TEST(PerformanceEngine, RejectsZeroWorkers) {
    EXPECT_THROW(
        performance_engine::PerformanceEngine({
            0,
            4096,
            256,
            64,
            performance_engine::AccessPattern::CacheLocal
        }),
        std::invalid_argument
    );
}

TEST(PerformanceEngine, BatchedQueuePreservesDeterministicChecksum) {
    performance_engine::PerformanceEngine original_engine({
        8,
        100000,
        256,
        1024,
        performance_engine::AccessPattern::CacheLocal,
        1
    });

    performance_engine::PerformanceEngine optimized_engine({
        8,
        100000,
        256,
        1024,
        performance_engine::AccessPattern::CacheLocal,
        8
    });

    const auto original =
        original_engine.run_multithreaded();

    const auto optimized =
        optimized_engine.run_multithreaded();

    EXPECT_EQ(original.checksum, optimized.checksum);
    EXPECT_EQ(
        original.tasks_processed,
        optimized.tasks_processed
    );
}

TEST(PerformanceEngine, BatchedQueueReducesLockAcquisitions) {
    performance_engine::PerformanceEngine original_engine({
        8,
        100000,
        256,
        1024,
        performance_engine::AccessPattern::CacheLocal,
        1
    });

    performance_engine::PerformanceEngine optimized_engine({
        8,
        100000,
        256,
        1024,
        performance_engine::AccessPattern::CacheLocal,
        8
    });

    const auto original =
        original_engine.run_multithreaded();

    const auto optimized =
        optimized_engine.run_multithreaded();

    EXPECT_LT(
        optimized.lock_acquisitions,
        original.lock_acquisitions
    );
}

TEST(PerformanceEngine, RejectsZeroQueueBatchSize) {
    EXPECT_THROW(
        performance_engine::PerformanceEngine({
            4,
            4096,
            256,
            64,
            performance_engine::AccessPattern::CacheLocal,
            0
        }),
        std::invalid_argument
    );
}
