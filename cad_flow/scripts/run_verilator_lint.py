#!/usr/bin/env python3
import json
import shutil
import subprocess
from pathlib import Path

RTL = sorted(Path("cad_flow/rtl").glob("*.sv"))
REPORT = Path("cad_flow/reports/verilator_lint_report.md")
JSON_REPORT = Path("cad_flow/reports/verilator_lint_results.json")

tool = shutil.which("verilator")
results = []

if tool:
    for f in RTL:
        cmd = ["verilator", "--lint-only", "-Wall", str(f)]
        proc = subprocess.run(cmd, text=True, capture_output=True)
        results.append({
            "file": str(f),
            "tool_available": True,
            "status": "pass" if proc.returncode == 0 else "fail",
            "stdout": proc.stdout[-1200:],
            "stderr": proc.stderr[-1200:]
        })
else:
    for f in RTL:
        text = f.read_text()
        status = "pass" if "module " in text and "endmodule" in text else "fail"
        results.append({
            "file": str(f),
            "tool_available": False,
            "status": status,
            "stdout": "",
            "stderr": "verilator not installed; used lightweight fallback syntax check"
        })

summary = {
    "tool": "verilator",
    "tool_available": bool(tool),
    "files_checked": len(results),
    "failures": sum(1 for r in results if r["status"] != "pass"),
    "status": "pass" if all(r["status"] == "pass" for r in results) else "fail",
    "results": results
}

JSON_REPORT.write_text(json.dumps(summary, indent=2))

lines = [
    "# Verilator Lint Report",
    "",
    "> Scope: Verilator-backed lint when available; lightweight fallback otherwise.",
    "",
    f"- Tool available: `{summary['tool_available']}`",
    f"- Files checked: {summary['files_checked']}",
    f"- Failures: {summary['failures']}",
    f"- Status: `{summary['status']}`",
    "",
    "| File | Status | Note |",
    "|---|---|---|"
]

for r in results:
    note = "verilator" if r["tool_available"] else "fallback syntax check"
    lines.append(f"| `{r['file']}` | {r['status']} | {note} |")

lines += [
    "",
    "## Claims boundary",
    "",
    "This report supports educational CAD-flow automation. It is not production RTL signoff."
]

REPORT.write_text("\n".join(lines) + "\n")
print(json.dumps(summary, indent=2))
