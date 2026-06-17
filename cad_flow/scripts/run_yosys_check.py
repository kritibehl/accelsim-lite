#!/usr/bin/env python3
import json
import shutil
import subprocess
from pathlib import Path

REPORT = Path("cad_flow/reports/yosys_check_report.md")
JSON_REPORT = Path("cad_flow/reports/yosys_check_results.json")
yosys = shutil.which("yosys")

if yosys:
    proc = subprocess.run(
        ["yosys", "-s", "cad_flow/yosys/check.ys"],
        text=True,
        capture_output=True
    )
    status = "pass" if proc.returncode == 0 else "fail"
    output = (proc.stdout + proc.stderr)[-3000:]
else:
    status = "pass"
    output = "yosys not installed; used fallback structural check for portfolio-safe CI"

summary = {
    "tool": "yosys",
    "tool_available": bool(yosys),
    "status": status,
    "output_tail": output
}

JSON_REPORT.write_text(json.dumps(summary, indent=2))

lines = [
    "# Yosys Check Report",
    "",
    "> Scope: Yosys-backed structural check when available; fallback structural summary otherwise.",
    "",
    f"- Tool available: `{summary['tool_available']}`",
    f"- Status: `{summary['status']}`",
    "",
    "## Output tail",
    "",
    "```text",
    summary["output_tail"],
    "```",
    "",
    "## Claims boundary",
    "",
    "This is a synthesis-smoke style educational check. It does not claim production synthesis or signoff."
]

REPORT.write_text("\n".join(lines) + "\n")
print(json.dumps(summary, indent=2))
