#include <gtest/gtest.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

TEST(OSConcepts, AtomicCounterPreservesExpectedCount) {
    constexpr int threads = 4;
    constexpr int iterations = 10000;
    std::atomic<int> counter{0};

    std::vector<std::thread> workers;
    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : workers) t.join();

    EXPECT_EQ(counter.load(), threads * iterations);
}

TEST(OSConcepts, MutexCounterPreservesExpectedCount) {
    constexpr int threads = 4;
    constexpr int iterations = 10000;
    int counter = 0;
    std::mutex m;

    std::vector<std::thread> workers;
    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<std::mutex> lock(m);
                counter++;
            }
        });
    }

    for (auto& t : workers) t.join();

    EXPECT_EQ(counter, threads * iterations);
}
