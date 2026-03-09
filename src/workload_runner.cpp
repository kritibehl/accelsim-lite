#include "accelsim/core/workload_runner.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace accelsim {
namespace {

std::vector<std::string> split(const std::string& input, char delimiter) {
    std::vector<std::string> out;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        out.push_back(item);
    }
    if (!input.empty() && input.back() == delimiter) {
        out.push_back("");
    }
    return out;
}

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

}  // namespace

std::string workload_name_from_path(const std::string& path) {
    return std::filesystem::path(path).stem().string();
}

std::vector<Instruction> load_workload_csv(const std::string& path, SimulatorConfig* config) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("failed to open workload: " + path);
    }

    if (config != nullptr) {
        config->workload_name = workload_name_from_path(path);
    }

    std::vector<Instruction> instructions;
    std::unordered_set<uint64_t> ids;
    std::string line;
    bool first_line = true;

    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (first_line) {
            first_line = false;
            if (line.rfind("id,", 0) == 0) {
                continue;
            }
        }

        auto cols = split(line, ',');
        if (cols.size() < 4) {
            throw std::runtime_error("expected at least 4 columns in line: " + line);
        }

        Instruction instruction;
        instruction.id = static_cast<uint64_t>(std::stoull(trim(cols[0])));
        if (!ids.insert(instruction.id).second) {
            throw std::runtime_error("duplicate instruction id: " + std::to_string(instruction.id));
        }

        instruction.type = parse_op_type(trim(cols[1]));
        instruction.execute_latency = static_cast<uint32_t>(std::stoul(trim(cols[2])));
        if (instruction.execute_latency == 0) {
            instruction.execute_latency = 1;
        }
        instruction.memory_bytes = static_cast<uint32_t>(std::stoul(trim(cols[3])));

        const auto deps_raw = cols.size() >= 5 ? trim(cols[4]) : "";
        if (!deps_raw.empty() && deps_raw != "0") {
            const char sep = (deps_raw.find('|') != std::string::npos) ? '|' : ';';
            for (const auto& token : split(deps_raw, sep)) {
                const auto dep = trim(token);
                if (!dep.empty() && dep != "0") {
                    instruction.deps.push_back(static_cast<uint64_t>(std::stoull(dep)));
                }
            }
        }

        instructions.push_back(instruction);
    }

    for (const auto& instruction : instructions) {
        for (uint64_t dep : instruction.deps) {
            if (ids.find(dep) == ids.end()) {
                throw std::runtime_error("missing dependency id: " + std::to_string(dep));
            }
        }
    }

    return instructions;
}

}  // namespace accelsim
