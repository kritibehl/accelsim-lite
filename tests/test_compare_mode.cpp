#include <gtest/gtest.h>
#include <vector>
#include "accelsim/core/simulator.hpp"
#include "accelsim/model/config.hpp"
#include "accelsim/model/instruction.hpp"

TEST(CompareMode, LowerMemoryPortsIncreasePressure) {
    using namespace accelsim;

    std::vector<Instruction> workload = {
        {1, OpType::Load, 8, 64, {}, Stage::Fetch, 0, 0, 0},
        {2, OpType::Load, 8, 64, {}, Stage::Fetch, 0, 0, 0},
        {3, OpType::Load, 8, 64, {}, Stage::Fetch, 0, 0, 0},
        {4, OpType::Load, 8, 64, {}, Stage::Fetch, 0, 0, 0}
    };

    SimulatorConfig fast;
    fast.workload_name = "fast";
    fast.memory_ports = 2;

    SimulatorConfig slow = fast;
    slow.workload_name = "slow";
    slow.memory_ports = 1;

    const auto fast_result = run_workload(workload, fast);
    const auto slow_result = run_workload(workload, slow);

    EXPECT_GT(slow_result.stats.total_cycles, fast_result.stats.total_cycles);
    EXPECT_GE(slow_result.stats.stall_counts.at("NoMemoryPort"),
              fast_result.stats.stall_counts.at("NoMemoryPort"));
}
