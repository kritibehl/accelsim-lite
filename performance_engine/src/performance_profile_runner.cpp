#include "performance_engine.h"

#include <chrono>
#include <cstdint>
#include <iostream>

int main() {
    constexpr auto profile_duration =
        std::chrono::seconds(15);

    const auto deadline =
        std::chrono::steady_clock::now() + profile_duration;

    std::uint64_t checksum = 0;
    std::uint64_t runs = 0;

    while (std::chrono::steady_clock::now() < deadline) {
        performance_engine::PerformanceEngine engine({
            8,
            1'000'000,
            256,
            1024,
            performance_engine::AccessPattern::CacheLocal,
            8
        });

        const auto result = engine.run_multithreaded();
        checksum ^= result.checksum;
        ++runs;
    }

    std::cout
        << "profile_runs=" << runs
        << " checksum=" << checksum
        << "\n";

    return 0;
}
