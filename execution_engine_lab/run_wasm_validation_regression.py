#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CASES = ROOT / "wasm_malformed_trace_regression_cases.json"
REPORT_MD = ROOT / "wasm_regression_report.md"
SUMMARY_JSON = ROOT / "wasm_regression_summary.json"

def validate(instructions):
    stack = []
    control = []

    steps = []

    def record(pc, opcode, before_stack, before_control, ok=True, error="none"):
        steps.append({
            "pc": pc,
            "opcode": opcode,
            "stack_height_before": before_stack,
            "stack_height_after": len(stack),
            "control_depth_before": before_control,
            "control_depth_after": len(control),
            "ok": ok,
            "error_reason": error,
        })

    for pc, ins in enumerate(instructions):
        opcode = ins["opcode"]
        operand = ins.get("operand", 0)
        before_stack = len(stack)
        before_control = len(control)

        if opcode == "i32.const":
            stack.append("i32")
            record(pc, opcode, before_stack, before_control)

        elif opcode == "local.get":
            if operand == 1:
                stack.append("unknown")
            else:
                stack.append("i32")
            record(pc, opcode, before_stack, before_control)

        elif opcode == "i32.add":
            if len(stack) < 2:
                record(pc, opcode, before_stack, before_control, False, "stack_underflow")
                return "stack_underflow", steps

            rhs = stack.pop()
            lhs = stack.pop()

            if lhs != "i32" or rhs != "i32":
                record(pc, opcode, before_stack, before_control, False, "type_mismatch")
                return "type_mismatch", steps

            stack.append("i32")
            record(pc, opcode, before_stack, before_control)

        elif opcode == "block":
            control.append("block")
            record(pc, opcode, before_stack, before_control)

        elif opcode == "loop":
            control.append("loop")
            record(pc, opcode, before_stack, before_control)

        elif opcode == "br_if":
            if not stack:
                record(pc, opcode, before_stack, before_control, False, "stack_underflow")
                return "stack_underflow", steps

            stack.pop()

            if operand < 0 or operand >= len(control):
                record(pc, opcode, before_stack, before_control, False, "invalid_branch_depth")
                return "invalid_branch_depth", steps

            record(pc, opcode, before_stack, before_control)

        elif opcode == "return":
            if not stack:
                record(pc, opcode, before_stack, before_control, False, "stack_underflow")
                return "stack_underflow", steps

            stack.pop()
            control.clear()
            record(pc, opcode, before_stack, before_control)

        else:
            record(pc, opcode, before_stack, before_control, False, "unknown_opcode")
            return "unknown_opcode", steps

    if control:
        steps.append({
            "pc": len(instructions),
            "opcode": "end_of_program",
            "stack_height_before": len(stack),
            "stack_height_after": len(stack),
            "control_depth_before": len(control),
            "control_depth_after": len(control),
            "ok": False,
            "error_reason": "unterminated_block",
        })
        return "unterminated_block", steps

    return "accepted", steps

def main():
    payload = json.loads(CASES.read_text())
    results = []

    for case in payload["cases"]:
        observed, steps = validate(case["instructions"])
        expected = case["expected_rejection_class"]
        match = observed == expected
        results.append({
            "name": case["name"],
            "expected_rejection_class": expected,
            "observed_rejection_class": observed,
            "match": match,
            "false_accept": observed == "accepted",
            "steps": steps,
        })

    total = len(results)
    correctly_rejected = sum(1 for r in results if r["match"])
    false_accepts = sum(1 for r in results if r["false_accept"])
    mismatches = total - correctly_rejected
    status = "pass" if correctly_rejected == total and false_accepts == 0 else "fail"

    summary = {
        "scope": "toy WebAssembly-style malformed-trace regression summary; not a real Wasm validator",
        "total_traces": total,
        "correctly_rejected_traces": correctly_rejected,
        "mismatches": mismatches,
        "false_accepts": false_accepts,
        "regression_status": status,
        "results": results,
    }

    SUMMARY_JSON.write_text(json.dumps(summary, indent=2))

    lines = []
    lines.append("# WebAssembly-Style Malformed Trace Regression Report\n")
    lines.append("> Scope: toy WebAssembly-style malformed-trace regression workflow. This is not a real Wasm validator, V8 implementation, JIT, or WebAssembly runtime.\n")
    lines.append("| Case | Expected Rejection | Observed Rejection | Match | False Accept |")
    lines.append("|---|---|---|---|---|")

    for r in results:
        lines.append(
            f"| {r['name']} | {r['expected_rejection_class']} | "
            f"{r['observed_rejection_class']} | {str(r['match']).lower()} | "
            f"{str(r['false_accept']).lower()} |"
        )

    lines.append("\n## Summary\n")
    lines.append(f"- Total traces: {total}")
    lines.append(f"- Correctly rejected traces: {correctly_rejected}")
    lines.append(f"- Mismatches: {mismatches}")
    lines.append(f"- False accepts: {false_accepts}")
    lines.append(f"- Regression status: `{status}`")

    lines.append("\n## Error classes covered\n")
    covered = sorted({r["expected_rejection_class"] for r in results})
    for err in covered:
        lines.append(f"- `{err}`")

    lines.append("\n## Safe interpretation\n")
    lines.append("This regression pack validates deterministic rejection behavior for malformed toy Wasm-style traces. It demonstrates validation-regression thinking without claiming production WebAssembly runtime or compiler experience.")

    REPORT_MD.write_text("\n".join(lines) + "\n")

    print(f"Generated {SUMMARY_JSON}")
    print(f"Generated {REPORT_MD}")
    print(json.dumps({
        "total_traces": total,
        "correctly_rejected_traces": correctly_rejected,
        "false_accepts": false_accepts,
        "regression_status": status,
    }, indent=2))

if __name__ == "__main__":
    main()
