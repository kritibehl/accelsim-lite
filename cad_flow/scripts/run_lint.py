#!/usr/bin/env python3
import json
from pathlib import Path

RTL_DIR = Path("cad_flow/rtl")
REPORT = Path("cad_flow/reports/lint_report.md")
JSON_REPORT = Path("cad_flow/reports/lint_results.json")

rules = {
    "module_declared": "module ",
    "endmodule_present": "endmodule"
}

results = []
for rtl in sorted(RTL_DIR.glob("*.sv")):
    text = rtl.read_text()
    issues = []

    for rule, token in rules.items():
        if token not in text:
            issues.append(rule)

    if "TODO" in text:
        issues.append("todo_marker_present")

    results.append({
        "file": str(rtl),
        "status": "pass" if not issues else "fail",
        "issues": issues
    })

summary = {
    "files_checked": len(results),
    "failures": sum(1 for r in results if r["status"] == "fail"),
    "status": "pass" if all(r["status"] == "pass" for r in results) else "fail",
    "results": results
}

JSON_REPORT.write_text(json.dumps(summary, indent=2))

lines = [
    "# RTL Lint Report",
    "",
    "> Scope: lightweight educational RTL lint checks for CAD-flow automation practice.",
    "",
    "| File | Status | Issues |",
    "|---|---|---|"
]

for r in results:
    lines.append(f"| `{r['file']}` | {r['status']} | {', '.join(r['issues']) if r['issues'] else '-'} |")

lines += [
    "",
    "## Summary",
    "",
    f"- Files checked: {summary['files_checked']}",
    f"- Failures: {summary['failures']}",
    f"- Status: `{summary['status']}`",
    "",
    "## Claims boundary",
    "",
    "This is a lightweight educational lint workflow, not a replacement for production RTL lint, synthesis, or EDA signoff tools."
]

REPORT.write_text("\n".join(lines) + "\n")
print(json.dumps(summary, indent=2))
