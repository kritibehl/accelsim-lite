#include <fstream>
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

struct ProgramCase {
    std::string name;
    std::vector<int> locals;
    std::vector<Instruction> baseline_program;
    std::vector<Instruction> optimized_program;
};

struct RunResult {
    int result = 0;
    bool returned = false;
    int dispatch_steps = 0;
    int branch_count = 0;
    int max_stack_depth = 0;
};

struct DiffResult {
    std::string name;
    RunResult baseline;
    RunResult optimized;
    bool semantically_equivalent = false;
    std::string mismatch_reason = "none";
    int dispatch_step_delta = 0;
};

Instruction ins(Opcode opcode, int operand = 0) {
    return Instruction{opcode, operand};
}

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

std::vector<ProgramCase> build_cases() {
    return {
        {
            "constant_fold_preserves_result",
            {},
            {
                ins(Opcode::PUSH_CONST, 2),
                ins(Opcode::PUSH_CONST, 3),
                ins(Opcode::ADD),
                ins(Opcode::PUSH_CONST, 4),
                ins(Opcode::MUL),
                ins(Opcode::RETURN)
            },
            {
                ins(Opcode::PUSH_CONST, 5),
                ins(Opcode::PUSH_CONST, 4),
                ins(Opcode::MUL),
                ins(Opcode::RETURN)
            }
        },
        {
            "load_constant_fold_preserves_result",
            {7},
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::PUSH_CONST, 10),
                ins(Opcode::PUSH_CONST, 2),
                ins(Opcode::MUL),
                ins(Opcode::ADD),
                ins(Opcode::RETURN)
            },
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::PUSH_CONST, 20),
                ins(Opcode::ADD),
                ins(Opcode::RETURN)
            }
        },
        {
            "branch_shape_preserved",
            {1},
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::JUMP_IF_ZERO, 6),
                ins(Opcode::PUSH_CONST, 6),
                ins(Opcode::PUSH_CONST, 7),
                ins(Opcode::MUL),
                ins(Opcode::RETURN),
                ins(Opcode::PUSH_CONST, 0),
                ins(Opcode::RETURN)
            },
            {
                ins(Opcode::LOAD, 0),
                ins(Opcode::JUMP_IF_ZERO, 4),
                ins(Opcode::PUSH_CONST, 42),
                ins(Opcode::RETURN),
                ins(Opcode::PUSH_CONST, 0),
                ins(Opcode::RETURN)
            }
        },
        {
            "intentional_mismatch_detected",
            {},
            {
                ins(Opcode::PUSH_CONST, 2),
                ins(Opcode::PUSH_CONST, 3),
                ins(Opcode::ADD),
                ins(Opcode::RETURN)
            },
            {
                ins(Opcode::PUSH_CONST, 6),
                ins(Opcode::RETURN)
            }
        }
    };
}

int pop(std::vector<int>& stack) {
    int v = stack.back();
    stack.pop_back();
    return v;
}

RunResult run_program(const std::vector<Instruction>& program,
                      const std::vector<int>& locals) {
    RunResult out;
    std::vector<int> stack;

    int pc = 0;
    while (pc >= 0 && pc < static_cast<int>(program.size())) {
        const auto& instruction = program[pc];
        out.dispatch_steps += 1;

        switch (instruction.opcode) {
            case Opcode::PUSH_CONST:
                stack.push_back(instruction.operand);
                pc += 1;
                break;

            case Opcode::LOAD:
                stack.push_back(locals.at(instruction.operand));
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
                out.branch_count += 1;
                int value = pop(stack);
                if (value == 0) {
                    pc = instruction.operand;
                } else {
                    pc += 1;
                }
                break;
            }

            case Opcode::RETURN:
                out.result = stack.empty() ? 0 : pop(stack);
                out.returned = true;
                pc = static_cast<int>(program.size());
                break;
        }

        if (static_cast<int>(stack.size()) > out.max_stack_depth) {
            out.max_stack_depth = static_cast<int>(stack.size());
        }
    }

    return out;
}

DiffResult compare_case(const ProgramCase& c) {
    DiffResult diff;
    diff.name = c.name;
    diff.baseline = run_program(c.baseline_program, c.locals);
    diff.optimized = run_program(c.optimized_program, c.locals);
    diff.dispatch_step_delta =
        diff.baseline.dispatch_steps - diff.optimized.dispatch_steps;

    if (diff.baseline.returned != diff.optimized.returned) {
        diff.semantically_equivalent = false;
        diff.mismatch_reason = "return_behavior_mismatch";
    } else if (diff.baseline.result != diff.optimized.result) {
        diff.semantically_equivalent = false;
        diff.mismatch_reason = "result_mismatch";
    } else {
        diff.semantically_equivalent = true;
    }

    return diff;
}

std::string json_bool(bool value) {
    return value ? "true" : "false";
}

void write_json(const std::vector<DiffResult>& results,
                const std::string& path) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"toy semantic-equivalence differential validation for stack-bytecode traces; not a production compiler test harness\",\n";
    out << "  \"results\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];

        out << "    {\n";
        out << "      \"case\": \"" << r.name << "\",\n";
        out << "      \"semantically_equivalent\": " << json_bool(r.semantically_equivalent) << ",\n";
        out << "      \"mismatch_reason\": \"" << r.mismatch_reason << "\",\n";
        out << "      \"baseline_result\": " << r.baseline.result << ",\n";
        out << "      \"optimized_result\": " << r.optimized.result << ",\n";
        out << "      \"baseline_dispatch_steps\": " << r.baseline.dispatch_steps << ",\n";
        out << "      \"optimized_dispatch_steps\": " << r.optimized.dispatch_steps << ",\n";
        out << "      \"dispatch_step_delta\": " << r.dispatch_step_delta << ",\n";
        out << "      \"baseline_branch_count\": " << r.baseline.branch_count << ",\n";
        out << "      \"optimized_branch_count\": " << r.optimized.branch_count << ",\n";
        out << "      \"baseline_max_stack_depth\": " << r.baseline.max_stack_depth << ",\n";
        out << "      \"optimized_max_stack_depth\": " << r.optimized.max_stack_depth << "\n";
        out << "    }";

        if (i + 1 != results.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
}

void write_markdown(const std::vector<DiffResult>& results,
                    const std::string& path) {
    std::ofstream out(path);

    out << "# Runtime Semantic Equivalence Report\n\n";
    out << "> Scope: toy differential validation for baseline vs optimized stack-bytecode traces. This is not a production compiler validation harness.\n\n";

    out << "| Case | Equivalent | Mismatch | Baseline Result | Optimized Result | Dispatch Delta | Branches B/O | Stack Depth B/O |\n";
    out << "|---|---|---|---:|---:|---:|---:|---:|\n";

    int pass = 0;
    int fail = 0;

    for (const auto& r : results) {
        if (r.semantically_equivalent) {
            pass += 1;
        } else {
            fail += 1;
        }

        out << "| " << r.name
            << " | " << (r.semantically_equivalent ? "true" : "false")
            << " | " << r.mismatch_reason
            << " | " << r.baseline.result
            << " | " << r.optimized.result
            << " | " << r.dispatch_step_delta
            << " | " << r.baseline.branch_count << "/" << r.optimized.branch_count
            << " | " << r.baseline.max_stack_depth << "/" << r.optimized.max_stack_depth
            << " |\n";
    }

    out << "\n## Summary\n\n";
    out << "- Equivalent optimized traces: " << pass << "\n";
    out << "- Mismatched optimized traces detected: " << fail << "\n";
    out << "- The harness catches result mismatches after optimization.\n";
    out << "- Dispatch-step deltas show the runtime cost effect of optimized traces.\n";

    out << "\n## Safe interpretation\n\n";
    out << "This lab demonstrates differential validation thinking: optimized traces must preserve baseline semantics while reducing dispatch work. It is intentionally small and does not claim production compiler or VM validation experience.\n";
}

int main() {
    const auto cases = build_cases();

    std::vector<DiffResult> results;
    for (const auto& c : cases) {
        results.push_back(compare_case(c));
    }

    write_json(results, "execution_engine_lab/semantic_equivalence_report.json");
    write_markdown(results, "execution_engine_lab/semantic_equivalence_report.md");

    std::cout << "Generated execution_engine_lab/semantic_equivalence_report.json\n";
    std::cout << "Generated execution_engine_lab/semantic_equivalence_report.md\n";

    return 0;
}
