#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

enum class Opcode {
    PUSH_CONST,
    LOAD,
    ADD,
    SUB,
    MUL,
    JUMP_IF_ZERO,
    RETURN
};

struct Instruction {
    Opcode opcode;
    int operand;
};

struct Workload {
    std::string name;
    std::string category;
    std::vector<int> locals;
    std::vector<Instruction> program;
};

struct Profile {
    std::string workload;
    std::string category;
    int result = 0;
    int instruction_count = 0;
    int dispatch_steps = 0;
    int branch_count = 0;
    int taken_branches = 0;
    int max_stack_depth = 0;
    double avg_basic_block_length = 0.0;
    std::map<std::string, int> opcode_frequency;
    std::string behavior_classification;
};

std::string opcode_name(Opcode opcode) {
    switch (opcode) {
        case Opcode::PUSH_CONST: return "PUSH_CONST";
        case Opcode::LOAD: return "LOAD";
        case Opcode::ADD: return "ADD";
        case Opcode::SUB: return "SUB";
        case Opcode::MUL: return "MUL";
        case Opcode::JUMP_IF_ZERO: return "JUMP_IF_ZERO";
        case Opcode::RETURN: return "RETURN";
    }
    return "UNKNOWN";
}

Instruction ins(Opcode opcode, int operand = 0) {
    return Instruction{opcode, operand};
}

std::vector<Workload> build_workloads() {
    return {
        Workload{
            "arithmetic_heavy",
            "arithmetic",
            {4, 7, 3},
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::LOAD, 1),
                ins(Opcode::MUL),
                ins(Opcode::LOAD, 2),
                ins(Opcode::ADD),
                ins(Opcode::PUSH_CONST, 2),
                ins(Opcode::MUL),
                ins(Opcode::RETURN)
            }
        },
        Workload{
            "branch_zero_fast_path",
            "branch_heavy",
            {0, 5},
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::JUMP_IF_ZERO, 6),
                ins(Opcode::LOAD, 1),
                ins(Opcode::PUSH_CONST, 10),
                ins(Opcode::MUL),
                ins(Opcode::RETURN),
                ins(Opcode::PUSH_CONST, 42),
                ins(Opcode::RETURN)
            }
        },
        Workload{
            "mixed_control_arithmetic",
            "mixed",
            {3, 2},
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::LOAD, 1),
                ins(Opcode::SUB),
                ins(Opcode::JUMP_IF_ZERO, 9),
                ins(Opcode::LOAD, 0),
                ins(Opcode::LOAD, 1),
                ins(Opcode::ADD),
                ins(Opcode::PUSH_CONST, 4),
                ins(Opcode::MUL),
                ins(Opcode::RETURN),
                ins(Opcode::PUSH_CONST, 0),
                ins(Opcode::RETURN)
            }
        }
    };
}

int pop(std::vector<int>& stack) {
    int value = stack.back();
    stack.pop_back();
    return value;
}

Profile run_workload(const Workload& workload) {
    Profile profile;
    profile.workload = workload.name;
    profile.category = workload.category;

    std::vector<int> stack;
    int pc = 0;
    int current_basic_block = 0;
    std::vector<int> block_lengths;

    while (pc >= 0 && pc < static_cast<int>(workload.program.size())) {
        const auto& instruction = workload.program[pc];

        profile.instruction_count += 1;
        profile.dispatch_steps += 1;
        profile.opcode_frequency[opcode_name(instruction.opcode)] += 1;
        current_basic_block += 1;

        switch (instruction.opcode) {
            case Opcode::PUSH_CONST:
                stack.push_back(instruction.operand);
                pc += 1;
                break;

            case Opcode::LOAD:
                stack.push_back(workload.locals.at(instruction.operand));
                pc += 1;
                break;

            case Opcode::ADD: {
                int rhs = pop(stack);
                int lhs = pop(stack);
                stack.push_back(lhs + rhs);
                pc += 1;
                break;
            }

            case Opcode::SUB: {
                int rhs = pop(stack);
                int lhs = pop(stack);
                stack.push_back(lhs - rhs);
                pc += 1;
                break;
            }

            case Opcode::MUL: {
                int rhs = pop(stack);
                int lhs = pop(stack);
                stack.push_back(lhs * rhs);
                pc += 1;
                break;
            }

            case Opcode::JUMP_IF_ZERO: {
                profile.branch_count += 1;
                int value = pop(stack);
                block_lengths.push_back(current_basic_block);
                current_basic_block = 0;

                if (value == 0) {
                    profile.taken_branches += 1;
                    pc = instruction.operand;
                } else {
                    pc += 1;
                }
                break;
            }

            case Opcode::RETURN:
                profile.result = stack.empty() ? 0 : pop(stack);
                block_lengths.push_back(current_basic_block);
                pc = static_cast<int>(workload.program.size());
                break;
        }

        profile.max_stack_depth = std::max(
            profile.max_stack_depth,
            static_cast<int>(stack.size())
        );
    }

    if (!block_lengths.empty()) {
        double total = 0.0;
        for (int length : block_lengths) {
            total += static_cast<double>(length);
        }
        profile.avg_basic_block_length = total / block_lengths.size();
    }

    const double branch_ratio =
        profile.instruction_count == 0
            ? 0.0
            : static_cast<double>(profile.branch_count) /
                  static_cast<double>(profile.instruction_count);

    if (branch_ratio >= 0.15) {
        profile.behavior_classification = "branch-heavy-dispatch";
    } else if (profile.opcode_frequency["MUL"] >= 2 ||
               profile.opcode_frequency["ADD"] >= 2) {
        profile.behavior_classification = "arithmetic-heavy-dispatch";
    } else {
        profile.behavior_classification = "mixed-dispatch";
    }

    return profile;
}

std::string json_escape(const std::string& input) {
    std::ostringstream out;
    for (char c : input) {
        if (c == '"') {
            out << "\\\"";
        } else {
            out << c;
        }
    }
    return out.str();
}

void write_workloads_json(const std::vector<Workload>& workloads,
                          const std::string& path) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"toy stack-bytecode workloads for interpreter dispatch analysis; not a compiler or JIT\",\n";
    out << "  \"workloads\": [\n";

    for (std::size_t i = 0; i < workloads.size(); ++i) {
        const auto& workload = workloads[i];

        out << "    {\n";
        out << "      \"name\": \"" << workload.name << "\",\n";
        out << "      \"category\": \"" << workload.category << "\",\n";
        out << "      \"instruction_count\": " << workload.program.size() << ",\n";
        out << "      \"program\": [\n";

        for (std::size_t j = 0; j < workload.program.size(); ++j) {
            const auto& instruction = workload.program[j];
            out << "        {\"opcode\": \"" << opcode_name(instruction.opcode)
                << "\", \"operand\": " << instruction.operand << "}";

            if (j + 1 != workload.program.size()) {
                out << ",";
            }

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (i + 1 != workloads.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
}

void write_frequency_json(const std::vector<Profile>& profiles,
                          const std::string& path) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"opcode-frequency and dispatch-profile report for toy stack bytecode interpreter\",\n";
    out << "  \"profiles\": [\n";

    for (std::size_t i = 0; i < profiles.size(); ++i) {
        const auto& p = profiles[i];

        out << "    {\n";
        out << "      \"workload\": \"" << json_escape(p.workload) << "\",\n";
        out << "      \"category\": \"" << p.category << "\",\n";
        out << "      \"result\": " << p.result << ",\n";
        out << "      \"instruction_count\": " << p.instruction_count << ",\n";
        out << "      \"dispatch_steps\": " << p.dispatch_steps << ",\n";
        out << "      \"branch_count\": " << p.branch_count << ",\n";
        out << "      \"taken_branches\": " << p.taken_branches << ",\n";
        out << "      \"max_stack_depth\": " << p.max_stack_depth << ",\n";
        out << "      \"avg_basic_block_length\": " << std::fixed << std::setprecision(2)
            << p.avg_basic_block_length << ",\n";
        out << "      \"behavior_classification\": \"" << p.behavior_classification << "\",\n";
        out << "      \"opcode_frequency\": {\n";

        std::size_t n = 0;
        for (const auto& [opcode, count] : p.opcode_frequency) {
            out << "        \"" << opcode << "\": " << count;
            if (++n != p.opcode_frequency.size()) {
                out << ",";
            }
            out << "\n";
        }

        out << "      }\n";
        out << "    }";

        if (i + 1 != profiles.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
}

void write_markdown(const std::vector<Profile>& profiles,
                    const std::string& path) {
    std::ofstream out(path);

    out << "# Interpreter Dispatch Report\n\n";
    out << "> Scope: toy C++ stack-bytecode interpreter for execution-engine dispatch analysis. This is not a compiler, JIT, V8 engine, or WebAssembly runtime.\n\n";

    out << "| Workload | Category | Result | Instructions | Dispatch Steps | Branches | Taken Branches | Avg Basic Block | Max Stack | Classification |\n";
    out << "|---|---|---:|---:|---:|---:|---:|---:|---:|---|\n";

    for (const auto& p : profiles) {
        out << "| " << p.workload
            << " | " << p.category
            << " | " << p.result
            << " | " << p.instruction_count
            << " | " << p.dispatch_steps
            << " | " << p.branch_count
            << " | " << p.taken_branches
            << " | " << std::fixed << std::setprecision(2) << p.avg_basic_block_length
            << " | " << p.max_stack_depth
            << " | " << p.behavior_classification
            << " |\n";
    }

    out << "\n## Opcode mix\n\n";

    for (const auto& p : profiles) {
        out << "### " << p.workload << "\n\n";
        out << "| Opcode | Count |\n";
        out << "|---|---:|\n";

        for (const auto& [opcode, count] : p.opcode_frequency) {
            out << "| " << opcode << " | " << count << " |\n";
        }

        out << "\n";
    }

    out << "## Interpretation\n\n";
    out << "- Arithmetic-heavy traces spend most dispatches on stack operations such as LOAD, ADD, MUL, and PUSH_CONST.\n";
    out << "- Branch-heavy traces introduce control-flow dispatch and shorter basic-block regions.\n";
    out << "- The dispatch-step count acts as a simple interpreter overhead proxy.\n";
    out << "- This lab demonstrates execution-engine reasoning without claiming compiler, JIT, V8, or WebAssembly implementation experience.\n";
}

int main() {
    const auto workloads = build_workloads();

    std::vector<Profile> profiles;
    for (const auto& workload : workloads) {
        profiles.push_back(run_workload(workload));
    }

    write_workloads_json(workloads, "execution_engine_lab/bytecode_workloads.json");
    write_frequency_json(profiles, "execution_engine_lab/opcode_frequency_report.json");
    write_markdown(profiles, "execution_engine_lab/interpreter_dispatch_report.md");

    std::cout << "Generated execution_engine_lab/bytecode_workloads.json\n";
    std::cout << "Generated execution_engine_lab/opcode_frequency_report.json\n";
    std::cout << "Generated execution_engine_lab/interpreter_dispatch_report.md\n";

    return 0;
}
