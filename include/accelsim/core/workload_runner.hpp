#pragma once
#include <string>
#include <vector>
#include "accelsim/model/config.hpp"
#include "accelsim/model/instruction.hpp"

namespace accelsim {

std::vector<Instruction> load_workload_csv(const std::string& path, SimulatorConfig* config = nullptr);
std::string workload_name_from_path(const std::string& path);

}  // namespace accelsim
