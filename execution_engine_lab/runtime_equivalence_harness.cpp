#include <fstream>
#include <iostream>
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

struct TraceCase {
    std::string name;
    std::vector<int> locals;
    std::vector<Instruction> baseline;
    std::vector<Instruction> optimized;
    std::vector<Instruction> deopt_fallback;
    bool expect_error = false;
    std::string expected_error = "none";
    std::string notes;
};

struct RunResult {
    bool returned = false;
    bool error = false;
    std::string error_category = "none";
    int return_value = 0;
    int final_stack_depth = 0;
    int branch_taken_count = 0;
    std::vector<int> final_stack;
};

struct EquivalenceResult {
    std::string trace;
    RunResult baseline;
    RunResult optimized;
    RunResult deopt_fallback;
    bool optimized_matches = false;
    bool deopt_matches = false;
    bool invalid_rejection_preserved = false;
    bool pass = false;
    std::string notes;
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

std::vector<TraceCase> build_cases() {
    return {
        {
            "arithmetic_constant_fold_equivalence",
            {},
            {ins(Opcode::PUSH_CONST, 2), ins(Opcode::PUSH_CONST, 3), ins(Opcode::ADD), ins(Opcode::RETURN)},
            {ins(Opcode::PUSH_CONST, 5), ins(Opcode::RETURN)},
            {ins(Opcode::PUSH_CONST, 5), ins(Opcode::RETURN)},
            false,
            "none",
            "constant folding preserved return value"
        },
        {
            "branch_taken_equivalence",
            {0},
            {ins(Opcode::LOAD, 0), ins(Opcode::JUMP_IF_ZERO, 4), ins(Opcode::PUSH_CONST, 9), ins(Opcode::RETURN), ins(Opcode::PUSH_CONST, 1), ins(Opcode::RETURN)},
            {ins(Opcode::LOAD, 0), ins(Opcode::JUMP_IF_ZERO, 4), ins(Opcode::PUSH_CONST, 9), ins(Opcode::RETURN), ins(Opcode::PUSH_CONST, 1), ins(Opcode::RETURN)},
            {ins(Opcode::LOAD, 0), ins(Opcode::JUMP_IF_ZERO, 4), ins(Opcode::PUSH_CONST, 9), ins(Opcode::RETURN), ins(Opcode::PUSH_CONST, 1), ins(Opcode::RETURN)},
            false,
            "none",
            "taken branch result preserved"
        },
        {
            "branch_not_taken_equivalence",
            {5},
            {ins(Opcode::LOAD, 0), ins(Opcode::JUMP_IF_ZERO, 4), ins(Opcode::PUSH_CONST, 9), ins(Opcode::RETURN), ins(Opcode::PUSH_CONST, 1), ins(Opcode::RETURN)},
            {ins(Opcode::LOAD, 0), ins(Opcode::JUMP_IF_ZERO, 4), ins(Opcode::PUSH_CONST, 9), ins(Opcode::RETURN), ins(Opcode::PUSH_CONST, 1), ins(Opcode::RETURN)},
            {ins(Opcode::LOAD, 0), ins(Opcode::JUMP_IF_ZERO, 4), ins(Opcode::PUSH_CONST, 9), ins(Opcode::RETURN), ins(Opcode::PUSH_CONST, 1), ins(Opcode::RETURN)},
            false,
            "none",
            "not-taken branch result preserved"
        },
        {
            "hot_trace_optimized_equivalence",
            {7},
            {ins(Opcode::LOAD, 0), ins(Opcode::PUSH_CONST, 10), ins(Opcode::PUSH_CONST, 2), ins(Opcode::MUL), ins(Opcode::ADD), ins(Opcode::RETURN)},
            {ins(Opcode::LOAD, 0), ins(Opcode::PUSH_CONST, 20), ins(Opcode::ADD), ins(Opcode::RETURN)},
            {ins(Opcode::LOAD, 0), ins(Opcode::PUSH_CONST, 20), ins(Opcode::ADD), ins(Opcode::RETURN)},
            false,
            "none",
            "hot optimized arithmetic path preserved result"
        },
        {
            "guard_failure_deopt_equivalence",
            {4},
            {ins(Opcode::LOAD, 0), ins(Opcode::PUSH_CONST, 3), ins(Opcode::ADD), ins(Opcode::RETURN)},
            {ins(Opcode::PUSH_CONST, 999), ins(Opcode::RETURN)},
            {ins(Opcode::LOAD, 0), ins(Opcode::PUSH_CONST, 3), ins(Opcode::ADD), ins(Opcode::RETURN)},
            false,
            "none",
            "optimized path would be unsafe, deopt fallback preserves baseline output"
        },
        {
            "invalid_trace_rejected_consistently",
            {},
            {ins(Opcode::PUSH_CONST, 1), ins(Opcode::ADD), ins(Opcode::RETURN)},
            {ins(Opcode::PUSH_CONST, 1), ins(Opcode::ADD), ins(Opcode::RETURN)},
            {ins(Opcode::PUSH_CONST, 1), ins(Opcode::ADD), ins(Opcode::RETURN)},
            true,
            "stack_underflow",
            "malformed trace rejected consistently"
        }
    };
}

int pop(std::vector<int>& stack, RunResult& result) {
    if (stack.empty()) {
        result.error = true;
        result.error_category = "stack_underflow";
        return 0;
    }

    int value = stack.back();
    stack.pop_back();
    return value;
}

RunResult run_program(const std::vector<Instruction>& program,
                      const std::vector<int>& locals) {
    RunResult result;
    std::vector<int> stack;
    int pc = 0;

    while (pc >= 0 && pc < static_cast<int>(program.size()) && !result.error) {
        const auto& instruction = program[pc];

        switch (instruction.opcode) {
            case Opcode::PUSH_CONST:
                stack.push_back(instruction.operand);
                pc += 1;
                break;

            case Opcode::LOAD:
                if (instruction.operand < 0 || instruction.operand >= static_cast<int>(locals.size())) {
                    result.error = true;
                    result.error_category = "invalid_local";
                    break;
                }
                stack.push_back(locals[instruction.operand]);
                pc += 1;
                break;

            case Opcode::ADD: {
                int rhs = pop(stack, result);
                int lhs = pop(stack, result);
                if (!result.error) stack.push_back(lhs + rhs);
                pc += 1;
                break;
            }

            case Opcode::SUB: {
                int rhs = pop(stack, result);
                int lhs = pop(stack, result);
                if (!result.error) stack.push_back(lhs - rhs);
                pc += 1;
                break;
            }

            case Opcode::MUL: {
                int rhs = pop(stack, result);
                int lhs = pop(stack, result);
                if (!result.error) stack.push_back(lhs * rhs);
                pc += 1;
                break;
            }

            case Opcode::JUMP_IF_ZERO: {
                int value = pop(stack, result);
                if (result.error) break;
                if (value == 0) {
                    result.branch_taken_count += 1;
                    pc = instruction.operand;
                } else {
                    pc += 1;
                }
                break;
            }

            case Opcode::RETURN:
                result.return_value = pop(stack, result);
                if (!result.error) result.returned = true;
                pc = static_cast<int>(program.size());
                break;
        }
    }

    result.final_stack = stack;
    result.final_stack_depth = static_cast<int>(stack.size());
    return result;
}

bool same_semantics(const RunResult& a, const RunResult& b) {
    return a.returned == b.returned &&
           a.error == b.error &&
           a.error_category == b.error_category &&
           a.return_value == b.return_value &&
           a.final_stack == b.final_stack &&
           a.branch_taken_count == b.branch_taken_count &&
           a.final_stack_depth == b.final_stack_depth;
}

EquivalenceResult compare_case(const TraceCase& c) {
    EquivalenceResult r;
    r.trace = c.name;
    r.baseline = run_program(c.baseline, c.locals);
    r.optimized = run_program(c.optimized, c.locals);
    r.deopt_fallback = run_program(c.deopt_fallback, c.locals);
    r.optimized_matches = same_semantics(r.baseline, r.optimized);
    r.deopt_matches = same_semantics(r.baseline, r.deopt_fallback);
    r.invalid_rejection_preserved =
        c.expect_error &&
        r.baseline.error &&
        r.optimized.error &&
        r.deopt_fallback.error &&
        r.baseline.error_category == c.expected_error &&
        r.optimized.error_category == c.expected_error &&
        r.deopt_fallback.error_category == c.expected_error;

    if (c.expect_error) {
        r.pass = r.invalid_rejection_preserved;
    } else if (c.name == "guard_failure_deopt_equivalence") {
        r.pass = !r.optimized_matches && r.deopt_matches;
    } else {
        r.pass = r.optimized_matches && r.deopt_matches;
    }

    r.notes = c.notes;
    return r;
}

std::string bool_json(bool v) {
    return v ? "true" : "false";
}

void write_json(const std::vector<TraceCase>& cases,
                const std::vector<EquivalenceResult>& results) {
    std::ofstream cases_json("execution_engine_lab/equivalence_trace_cases.json");
    cases_json << "{\n";
    cases_json << "  \"scope\": \"toy runtime equivalence cases for baseline, optimized, and deopt fallback paths\",\n";
    cases_json << "  \"case_count\": " << cases.size() << ",\n";
    cases_json << "  \"cases\": [\n";

    for (std::size_t i = 0; i < cases.size(); ++i) {
        cases_json << "    {\"name\": \"" << cases[i].name << "\", \"notes\": \"" << cases[i].notes << "\"}";
        if (i + 1 != cases.size()) cases_json << ",";
        cases_json << "\n";
    }

    cases_json << "  ]\n";
    cases_json << "}\n";

    int total = 0;
    int matches = 0;
    int mismatches = 0;
    int deopt_matches = 0;
    int invalid_preserved = 0;

    for (const auto& r : results) {
        total += 1;
        if (r.pass) matches += 1;
        if (!r.pass) mismatches += 1;
        if (r.deopt_matches) deopt_matches += 1;
        if (r.invalid_rejection_preserved) invalid_preserved += 1;
    }

    std::ofstream summary("execution_engine_lab/runtime_equivalence_summary.json");
    summary << "{\n";
    summary << "  \"scope\": \"toy differential runtime validation summary; not production VM/compiler validation\",\n";
    summary << "  \"total_cases\": " << total << ",\n";
    summary << "  \"semantic_matches\": " << matches << ",\n";
    summary << "  \"semantic_mismatches\": " << mismatches << ",\n";
    summary << "  \"deopt_fallback_matches\": " << deopt_matches << ",\n";
    summary << "  \"invalid_trace_rejections_preserved\": " << invalid_preserved << "\n";
    summary << "}\n";
}

void write_markdown(const std::vector<EquivalenceResult>& results) {
    std::ofstream out("execution_engine_lab/runtime_equivalence_report.md");

    out << "# Runtime Equivalence Report\n\n";
    out << "> Scope: toy differential runtime validation for baseline, optimized, and deoptimized stack-bytecode execution paths. This is not a production compiler, JIT, VM, V8, or WebAssembly validation harness.\n\n";

    out << "| Trace | Baseline Result/Error | Optimized Result/Error | Deopt Result/Error | Match? | Notes |\n";
    out << "|---|---|---|---|---|---|\n";

    for (const auto& r : results) {
        auto show = [](const RunResult& rr) {
            if (rr.error) return std::string("error:") + rr.error_category;
            return std::to_string(rr.return_value);
        };

        out << "| " << r.trace
            << " | " << show(r.baseline)
            << " | " << show(r.optimized)
            << " | " << show(r.deopt_fallback)
            << " | " << (r.pass ? "PASS" : "FAIL")
            << " | " << r.notes
            << " |\n";
    }

    out << "\n## What is compared\n\n";
    out << "- returned value\n";
    out << "- final stack contents\n";
    out << "- error category\n";
    out << "- branch outcome count\n";
    out << "- stack depth at exit\n";
    out << "- deopt fallback output correctness\n\n";

    out << "## Safe interpretation\n\n";
    out << "This lab demonstrates semantic-drift detection across baseline, optimized, and deoptimized toy runtime paths. It intentionally does not claim production compiler, VM, JIT, V8, or WebAssembly validation experience.\n";
}

int main() {
    const auto cases = build_cases();

    std::vector<EquivalenceResult> results;
    for (const auto& c : cases) {
        results.push_back(compare_case(c));
    }

    write_json(cases, results);
    write_markdown(results);

    std::cout << "Generated execution_engine_lab/equivalence_trace_cases.json\n";
    std::cout << "Generated execution_engine_lab/runtime_equivalence_summary.json\n";
    std::cout << "Generated execution_engine_lab/runtime_equivalence_report.md\n";
    return 0;
}
