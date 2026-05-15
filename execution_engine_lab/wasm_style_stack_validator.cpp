#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Instruction {
    std::string opcode;
    int operand = 0;
};

struct Case {
    std::string name;
    std::vector<Instruction> program;
};

struct ValidationStep {
    int pc = 0;
    std::string opcode;
    int stack_height_before = 0;
    int stack_height_after = 0;
    int control_depth_before = 0;
    int control_depth_after = 0;
    bool ok = true;
    std::string error_reason = "none";
};

struct ValidationResult {
    std::string name;
    bool valid = true;
    std::string error_classification = "none";
    std::vector<ValidationStep> steps;
};

std::vector<Case> build_cases() {
    return {
        {
            "valid_arithmetic_return",
            {
                {"i32.const", 1},
                {"i32.const", 2},
                {"i32.add", 0},
                {"return", 0}
            }
        },
        {
            "valid_block_branch",
            {
                {"block", 0},
                {"i32.const", 1},
                {"br_if", 0},
                {"i32.const", 2},
                {"return", 0}
            }
        },
        {
            "valid_loop_counter_shape",
            {
                {"loop", 0},
                {"local.get", 0},
                {"i32.const", 1},
                {"i32.add", 0},
                {"br_if", 0},
                {"i32.const", 0},
                {"return", 0}
            }
        },
        {
            "invalid_stack_underflow_add",
            {
                {"i32.const", 1},
                {"i32.add", 0},
                {"return", 0}
            }
        },
        {
            "invalid_type_mismatch_add",
            {
                {"local.get", 1},
                {"i32.const", 2},
                {"i32.add", 0},
                {"return", 0}
            }
        },
        {
            "invalid_branch_depth",
            {
                {"block", 0},
                {"i32.const", 1},
                {"br_if", 2},
                {"return", 0}
            }
        },
        {
            "unterminated_block",
            {
                {"block", 0},
                {"i32.const", 1},
                {"i32.const", 2},
                {"i32.add", 0}
            }
        }
    };
}

std::string json_escape(const std::string& s) {
    std::ostringstream out;
    for (char c : s) {
        if (c == '"') {
            out << "\\\"";
        } else {
            out << c;
        }
    }
    return out.str();
}

bool is_i32_value(const std::string& value_type) {
    return value_type == "i32";
}

ValidationResult validate_case(const Case& c) {
    ValidationResult result;
    result.name = c.name;

    std::vector<std::string> operand_stack;
    std::vector<std::string> control_stack;

    auto fail = [&](ValidationStep& step, const std::string& reason) {
        step.ok = false;
        step.error_reason = reason;
        result.valid = false;
        result.error_classification = reason;
    };

    for (std::size_t pc = 0; pc < c.program.size(); ++pc) {
        const auto& ins = c.program[pc];

        ValidationStep step;
        step.pc = static_cast<int>(pc);
        step.opcode = ins.opcode;
        step.stack_height_before = static_cast<int>(operand_stack.size());
        step.control_depth_before = static_cast<int>(control_stack.size());

        if (ins.opcode == "i32.const") {
            operand_stack.push_back("i32");
        } else if (ins.opcode == "local.get") {
            if (ins.operand == 1) {
                operand_stack.push_back("unknown");
            } else {
                operand_stack.push_back("i32");
            }
        } else if (ins.opcode == "i32.add") {
            if (operand_stack.size() < 2) {
                fail(step, "stack_underflow");
            } else {
                std::string rhs = operand_stack.back();
                operand_stack.pop_back();
                std::string lhs = operand_stack.back();
                operand_stack.pop_back();

                if (!is_i32_value(lhs) || !is_i32_value(rhs)) {
                    fail(step, "type_mismatch");
                } else {
                    operand_stack.push_back("i32");
                }
            }
        } else if (ins.opcode == "block") {
            control_stack.push_back("block");
        } else if (ins.opcode == "loop") {
            control_stack.push_back("loop");
        } else if (ins.opcode == "br_if") {
            if (operand_stack.empty()) {
                fail(step, "stack_underflow");
            } else {
                operand_stack.pop_back();

                if (ins.operand < 0 ||
                    ins.operand >= static_cast<int>(control_stack.size())) {
                    fail(step, "invalid_branch_depth");
                }
            }
        } else if (ins.opcode == "return") {
            if (operand_stack.empty()) {
                fail(step, "stack_underflow");
            } else {
                operand_stack.pop_back();
            }

            control_stack.clear();
        } else {
            fail(step, "unknown_opcode");
        }

        step.stack_height_after = static_cast<int>(operand_stack.size());
        step.control_depth_after = static_cast<int>(control_stack.size());
        result.steps.push_back(step);

        if (!result.valid) {
            return result;
        }
    }

    if (!control_stack.empty()) {
        ValidationStep step;
        step.pc = static_cast<int>(c.program.size());
        step.opcode = "end_of_program";
        step.stack_height_before = static_cast<int>(operand_stack.size());
        step.stack_height_after = static_cast<int>(operand_stack.size());
        step.control_depth_before = static_cast<int>(control_stack.size());
        step.control_depth_after = static_cast<int>(control_stack.size());
        fail(step, "unterminated_block");
        result.steps.push_back(step);
        return result;
    }

    return result;
}

void write_json(const std::vector<Case>& cases,
                const std::vector<ValidationResult>& results,
                const std::string& path) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"toy WebAssembly-style stack validation; not a real Wasm runtime or validator\",\n";
    out << "  \"cases\": [\n";

    for (std::size_t i = 0; i < cases.size(); ++i) {
        const auto& c = cases[i];
        out << "    {\n";
        out << "      \"name\": \"" << json_escape(c.name) << "\",\n";
        out << "      \"program\": [\n";

        for (std::size_t j = 0; j < c.program.size(); ++j) {
            out << "        {\"opcode\": \"" << c.program[j].opcode
                << "\", \"operand\": " << c.program[j].operand << "}";

            if (j + 1 != c.program.size()) {
                out << ",";
            }

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (i + 1 != cases.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ],\n";
    out << "  \"validation_results\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];

        out << "    {\n";
        out << "      \"name\": \"" << json_escape(r.name) << "\",\n";
        out << "      \"valid\": " << (r.valid ? "true" : "false") << ",\n";
        out << "      \"error_classification\": \"" << r.error_classification << "\",\n";
        out << "      \"steps\": [\n";

        for (std::size_t j = 0; j < r.steps.size(); ++j) {
            const auto& s = r.steps[j];

            out << "        {\n";
            out << "          \"pc\": " << s.pc << ",\n";
            out << "          \"opcode\": \"" << json_escape(s.opcode) << "\",\n";
            out << "          \"stack_height_before\": " << s.stack_height_before << ",\n";
            out << "          \"stack_height_after\": " << s.stack_height_after << ",\n";
            out << "          \"control_depth_before\": " << s.control_depth_before << ",\n";
            out << "          \"control_depth_after\": " << s.control_depth_after << ",\n";
            out << "          \"ok\": " << (s.ok ? "true" : "false") << ",\n";
            out << "          \"error_reason\": \"" << json_escape(s.error_reason) << "\"\n";
            out << "        }";

            if (j + 1 != r.steps.size()) {
                out << ",";
            }

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (i + 1 != results.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
}

void write_markdown(const std::vector<ValidationResult>& results,
                    const std::string& path) {
    std::ofstream out(path);

    out << "# WebAssembly-Style Stack Validation Report\n\n";
    out << "> Scope: toy WebAssembly-style stack validation for structured control-flow traces. This is not a real WebAssembly runtime, compiler, or validator.\n\n";

    out << "| Case | Valid | Error Classification | Steps Checked |\n";
    out << "|---|---|---|---:|\n";

    std::map<std::string, int> error_counts;
    int valid_count = 0;
    int invalid_count = 0;

    for (const auto& r : results) {
        out << "| " << r.name
            << " | " << (r.valid ? "true" : "false")
            << " | " << r.error_classification
            << " | " << r.steps.size()
            << " |\n";

        if (r.valid) {
            valid_count += 1;
        } else {
            invalid_count += 1;
            error_counts[r.error_classification] += 1;
        }
    }

    out << "\n## Stack-height and control-flow transitions\n\n";

    for (const auto& r : results) {
        out << "### " << r.name << "\n\n";
        out << "| PC | Opcode | Stack Before | Stack After | Control Before | Control After | OK | Error |\n";
        out << "|---:|---|---:|---:|---:|---:|---|---|\n";

        for (const auto& s : r.steps) {
            out << "| " << s.pc
                << " | " << s.opcode
                << " | " << s.stack_height_before
                << " | " << s.stack_height_after
                << " | " << s.control_depth_before
                << " | " << s.control_depth_after
                << " | " << (s.ok ? "true" : "false")
                << " | " << s.error_reason
                << " |\n";
        }

        out << "\n";
    }

    out << "## Summary\n\n";
    out << "- Valid traces: " << valid_count << "\n";
    out << "- Invalid traces: " << invalid_count << "\n";
    out << "- Error classes observed:\n";

    for (const auto& [error, count] : error_counts) {
        out << "  - `" << error << "`: " << count << "\n";
    }

    out << "\n## Interpretation\n\n";
    out << "- Operand-stack validation catches stack underflow before execution.\n";
    out << "- Type validation catches non-i32 values used by i32 arithmetic.\n";
    out << "- Structured control-flow validation catches invalid branch depths and unterminated blocks.\n";
    out << "- This lab demonstrates WebAssembly-adjacent validation reasoning without claiming real Wasm engine implementation.\n";
}

int main() {
    const auto cases = build_cases();

    std::vector<ValidationResult> results;
    for (const auto& c : cases) {
        results.push_back(validate_case(c));
    }

    write_json(cases, results, "execution_engine_lab/wasm_control_flow_cases.json");
    write_markdown(results, "execution_engine_lab/wasm_validation_report.md");

    std::cout << "Generated execution_engine_lab/wasm_control_flow_cases.json\n";
    std::cout << "Generated execution_engine_lab/wasm_validation_report.md\n";

    return 0;
}
