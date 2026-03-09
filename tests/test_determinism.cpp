#include <gtest/gtest.h>
#include <vector>
#include "accelsim/core/simulator.hpp"
#include "accelsim/model/config.hpp"
#include "accelsim/model/instruction.hpp"

TEST(Determinism, SameWorkloadSameSummary) {
    using namespace accelsim;

    std::vector<Instruction> workload = {
        {1, OpType::Compute, 3, 0, {}, Stage::Fetch, 0, 0, 0},
        {2, OpType::Load, 8, 64, {}, Stage::Fetch, 0, 0, 0},
        {3, OpType::Compute, 2, 0, {1, 2}, Stage::Fetch, 0, 0, 0},
        {4, OpType::Store, 6, 64, {3}, Stage::Fetch, 0, 0, 0}
    };

    SimulatorConfig config;
    config.workload_name = "determinism";

    const auto a = run_workload(workload, config);
    const auto b = run_workload(workload, config);

    EXPECT_EQ(a.stats.total_cycles, b.stats.total_cycles);
    EXPECT_EQ(a.stats.completed_ops, b.stats.completed_ops);
    EXPECT_DOUBLE_EQ(a.stats.throughput(), b.stats.throughput());
    EXPECT_DOUBLE_EQ(a.stats.average_latency(), b.stats.average_latency());
    EXPECT_EQ(a.stats.stall_counts, b.stats.stall_counts);
    EXPECT_EQ(a.stats.stage_busy_cycles, b.stats.stage_busy_cycles);
}
