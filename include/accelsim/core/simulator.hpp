#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "accelsim/model/config.hpp"
#include "accelsim/model/instruction.hpp"
#include "accelsim/model/stats.hpp"

namespace accelsim {

struct RunResult {
    SimulatorConfig config;
    Stats stats;
    std::vector<Instruction> instructions;
};

RunResult run_workload(const std::vector<Instruction>& workload, const SimulatorConfig& config);
std::string summarize(const RunResult& result);
std::string compare_runs(const RunResult& left, const RunResult& right);
void write_report(const RunResult& result, const std::filesystem::path& out_dir);

}  // namespace accelsim
