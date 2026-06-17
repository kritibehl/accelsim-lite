#!/usr/bin/env python3
import json
from pathlib import Path

OUT = Path("cad_flow/reports/changed_rtl_impact.json")
REPORT = Path("cad_flow/reports/changed_rtl_impact.md")

# Synthetic changed-file input for portfolio-safe CAD-flow impact analysis.
changed_files = [
    "cad_flow/rtl/fifo.sv",
    "cad_flow/rtl/register_file.sv"
]

impact_map = {
    "cad_flow/rtl/fifo.sv": ["fifo_push_pop", "fifo_full_empty", "fifo_ordering"],
    "cad_flow/rtl/arbiter.sv": ["arbiter_priority", "arbiter_no_double_grant"],
    "cad_flow/rtl/register_file.sv": ["register_file_read_write", "register_file_address_bounds"]
}

impacted_tests = sorted({test for f in changed_files for test in impact_map.get(f, [])})

summary = {
    "changed_files": changed_files,
    "impacted_tests": impacted_tests,
    "impact_count": len(impacted_tests),
    "recommendation": "run targeted RTL regression for changed modules"
}

OUT.write_text(json.dumps(summary, indent=2))

lines = [
    "# Changed RTL Impact Report",
    "",
    "| Changed RTL | Impacted tests |",
    "|---|---|"
]

for f in changed_files:
    lines.append(f"| `{f}` | {', '.join(impact_map.get(f, []))} |")

lines += [
    "",
    "## Recommendation",
    "",
    summary["recommendation"],
    "",
    "## Claims boundary",
    "",
    "This is an educational changed-file impact workflow for CAD automation practice."
]

REPORT.write_text("\n".join(lines) + "\n")
print(json.dumps(summary, indent=2))
