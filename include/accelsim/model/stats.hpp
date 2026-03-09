#pragma once
#include <cstdint>
#include <map>
#include <string>

namespace accelsim {

struct Stats {
    uint64_t total_cycles{0};
    uint64_t completed_ops{0};
    uint64_t total_latency{0};

    std::map<std::string, uint64_t> stage_busy_cycles;
    std::map<std::string, uint64_t> stall_counts;
    std::map<std::string, uint64_t> max_queue_occupancy;

    double throughput() const {
        return total_cycles == 0 ? 0.0
                                 : static_cast<double>(completed_ops) / static_cast<double>(total_cycles);
    }

    double average_latency() const {
        return completed_ops == 0 ? 0.0
                                  : static_cast<double>(total_latency) / static_cast<double>(completed_ops);
    }

    std::string top_bottleneck() const {
        std::string best{"None"};
        uint64_t best_value{0};
        for (const auto& [name, value] : stall_counts) {
            if (value > best_value) {
                best = name;
                best_value = value;
            }
        }
        return best;
    }
};

}  // namespace accelsim
