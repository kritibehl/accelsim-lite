#include <algorithm>
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
    std::vector<Instruction> before;
};

struct TraceStats {
    int instruction_count = 0;
    int dispatch_steps = 0;
    int branch_count = 0;
    int max_stack_depth_proxy = 0;
    std::map<std::string, int> opcode_frequency;
};

struct OptimizationResult {
    std::string workload;
    std::vector<Instruction> before;
    std::vector<Instruction> after;
    TraceStats before_stats;
    TraceStats after_stats;
    int folded_patterns = 0;
    int dispatch_step_reduction = 0;
    double dispatch_reduction_percent = 0.0;
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
            "constant_fold_add_mul",
            {
                ins(Opcode::PUSH_CONST, 2),
                ins(Opcode::PUSH_CONST, 3),
                ins(Opcode::ADD),
                ins(Opcode::PUSH_CONST, 4),
                ins(Opcode::MUL),
                ins(Opcode::RETURN)
            }
        },
        Workload{
            "mixed_load_and_constants",
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::PUSH_CONST, 10),
                ins(Opcode::PUSH_CONST, 2),
                ins(Opcode::MUL),
                ins(Opcode::ADD),
                ins(Opcode::RETURN)
            }
        },
        Workload{
            "branch_guarded_arithmetic",
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::JUMP_IF_ZERO, 8),
                ins(Opcode::PUSH_CONST, 6),
                ins(Opcode::PUSH_CONST, 7),
                ins(Opcode::MUL),
                ins(Opcode::PUSH_CONST, 3),
                ins(Opcode::ADD),
                ins(Opcode::RETURN),
                ins(Opcode::PUSH_CONST, 0),
                ins(Opcode::RETURN)
            }
        }
    };
}

TraceStats profile(const std::vector<Instruction>& program) {
    TraceStats stats;
    int stack_depth = 0;

    for (const auto& instruction : program) {
        stats.instruction_count += 1;
        stats.dispatch_steps += 1;
        stats.opcode_frequency[opcode_name(instruction.opcode)] += 1;

        switch (instruction.opcode) {
            case Opcode::PUSH_CONST:
            case Opcode::LOAD:
                stack_depth += 1;
                break;

            case Opcode::ADD:
            case Opcode::SUB:
            case Opcode::MUL:
                stack_depth = std::max(0, stack_depth - 1);
                break;

            case Opcode::JUMP_IF_ZERO:
                stats.branch_count += 1;
                stack_depth = std::max(0, stack_depth - 1);
                break;

            case Opcode::RETURN:
                stack_depth = std::max(0, stack_depth - 1);
                break;
        }

        stats.max_stack_depth_proxy = std::max(stats.max_stack_depth_proxy, stack_depth);
    }

    return stats;
}

bool is_arithmetic(Opcode opcode) {
    return opcode == Opcode::ADD || opcode == Opcode::SUB || opcode == Opcode::MUL;
}

int fold_value(int lhs, int rhs, Opcode opcode) {
    switch (opcode) {
        case Opcode::ADD:
            return lhs + rhs;
        case Opcode::SUB:
            return lhs - rhs;
        case Opcode::MUL:
            return lhs * rhs;
        default:
            return lhs;
    }
}

std::vector<Instruction> optimize_constants(const std::vector<Instruction>& program,
                                            int& folded_patterns) {
    std::vector<Instruction> optimized;

    for (std::size_t i = 0; i < program.size();) {
        if (i + 2 < program.size() &&
            program[i].opcode == Opcode::PUSH_CONST &&
            program[i + 1].opcode == Opcode::PUSH_CONST &&
            is_arithmetic(program[i + 2].opcode)) {
            int folded = fold_value(
                program[i].operand,
                program[i + 1].operand,
                program[i + 2].opcode
            );

            optimized.push_back(ins(Opcode::PUSH_CONST, folded));
            folded_patterns += 1;
            i += 3;
            continue;
        }

        optimized.push_back(program[i]);
        i += 1;
    }

    return optimized;
}

OptimizationResult optimize_workload(const Workload& workload) {
    OptimizationResult result;
    result.workload = workload.name;
    result.before = workload.before;

    int folded = 0;
    result.after = optimize_constants(workload.before, folded);
    result.folded_patterns = folded;

    result.before_stats = profile(result.before);
    result.after_stats = profile(result.after);
    result.dispatch_step_reduction =
        result.before_stats.dispatch_steps - result.after_stats.dispatch_steps;

    if (result.before_stats.dispatch_steps > 0) {
        result.dispatch_reduction_percent =
            100.0 * static_cast<double>(result.dispatch_step_reduction) /
            static_cast<double>(result.before_stats.dispatch_steps);
    }

    return result;
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

void write_program_json(std::ofstream& out,
                        const std::vector<Instruction>& program,
                        int indent) {
    std::string pad(indent, ' ');
    out << "[\n";

    for (std::size_t i = 0; i < program.size(); ++i) {
        out << pad << "  {\"opcode\": \"" << opcode_name(program[i].opcode)
            << "\", \"operand\": " << program[i].operand << "}";

        if (i + 1 != program.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << pad << "]";
}

void write_frequency_json(std::ofstream& out,
                          const std::map<std::string, int>& freq,
                          int indent) {
    std::string pad(indent, ' ');
    out << "{\n";

    std::size_t i = 0;
    for (const auto& [opcode, count] : freq) {
        out << pad << "  \"" << opcode << "\": " << count;

        if (++i != freq.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << pad << "}";
}

void write_json(const std::vector<OptimizationResult>& results,
                const std::string& path) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"toy bytecode optimization pass; not a compiler, JIT, V8, or WebAssembly runtime\",\n";
    out << "  \"optimization\": \"constant folding for PUSH_CONST/PUSH_CONST/arithmetic patterns\",\n";
    out << "  \"results\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];

        out << "    {\n";
        out << "      \"workload\": \"" << json_escape(r.workload) << "\",\n";
        out << "      \"folded_patterns\": " << r.folded_patterns << ",\n";
        out << "      \"before_instruction_count\": " << r.before_stats.instruction_count << ",\n";
        out << "      \"after_instruction_count\": " << r.after_stats.instruction_count << ",\n";
        out << "      \"dispatch_step_reduction\": " << r.dispatch_step_reduction << ",\n";
        out << "      \"dispatch_reduction_percent\": " << std::fixed << std::setprecision(2)
            << r.dispatch_reduction_percent << ",\n";
        out << "      \"before_branch_count\": " << r.before_stats.branch_count << ",\n";
        out << "      \"after_branch_count\": " << r.after_stats.branch_count << ",\n";
        out << "      \"before_max_stack_depth_proxy\": " << r.before_stats.max_stack_depth_proxy << ",\n";
        out << "      \"after_max_stack_depth_proxy\": " << r.after_stats.max_stack_depth_proxy << ",\n";
        out << "      \"before_opcode_frequency\": ";
        write_frequency_json(out, r.before_stats.opcode_frequency, 6);
        out << ",\n";
        out << "      \"after_opcode_frequency\": ";
        write_frequency_json(out, r.after_stats.opcode_frequency, 6);
        out << ",\n";
        out << "      \"before_program\": ";
        write_program_json(out, r.before, 6);
        out << ",\n";
        out << "      \"after_program\": ";
        write_program_json(out, r.after, 6);
        out << "\n";
        out << "    }";

        if (i + 1 != results.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
}

void write_markdown(const std::vector<OptimizationResult>& results,
                    const std::string& path) {
    std::ofstream out(path);

    out << "# Bytecode Optimization Report\n\n";
    out << "> Scope: toy bytecode optimization pass for interpreter-style execution traces. This is not a compiler, JIT, V8 engine, or WebAssembly runtime.\n\n";

    out << "| Workload | Folded Patterns | Instructions Before | Instructions After | Dispatch Reduction | Reduction % | Branches Before/After | Stack Depth Before/After |\n";
    out << "|---|---:|---:|---:|---:|---:|---:|---:|\n";

    for (const auto& r : results) {
        out << "| " << r.workload
            << " | " << r.folded_patterns
            << " | " << r.before_stats.instruction_count
            << " | " << r.after_stats.instruction_count
            << " | " << r.dispatch_step_reduction
            << " | " << std::fixed << std::setprecision(2)
            << r.dispatch_reduction_percent << "%"
            << " | " << r.before_stats.branch_count << "/" << r.after_stats.branch_count
            << " | " << r.before_stats.max_stack_depth_proxy << "/" << r.after_stats.max_stack_depth_proxy
            << " |\n";
    }

    out << "\n## Optimization behavior\n\n";
    out << "- Constant folding collapses `PUSH_CONST`, `PUSH_CONST`, arithmetic-op sequences into a single `PUSH_CONST`.\n";
    out << "- Arithmetic-heavy traces show the largest dispatch-step reduction.\n";
    out << "- Branch-heavy traces retain control-flow structure while still reducing constant arithmetic regions.\n";
    out << "- This demonstrates code-transformation reasoning without claiming production compiler optimization experience.\n";

    out << "\n## Safe interpretation\n\n";
    out << "The lab shows how a small transformation pass can reduce interpreter dispatch steps and alter opcode mix for toy bytecode traces. It is intended as execution-engine and compiler-adjacent practice, not a real JIT/compiler implementation.\n";
}

int main() {
    const auto workloads = build_workloads();

    std::vector<OptimizationResult> results;
    for (const auto& workload : workloads) {
        results.push_back(optimize_workload(workload));
    }

    write_json(results, "execution_engine_lab/optimized_trace_comparison.json");
    write_markdown(results, "execution_engine_lab/bytecode_optimization_report.md");

    std::cout << "Generated execution_engine_lab/optimized_trace_comparison.json\n";
    std::cout << "Generated execution_engine_lab/bytecode_optimization_report.md\n";

    return 0;
}
