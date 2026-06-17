#!/usr/bin/env python3
import json
from pathlib import Path

reports = Path("cad_flow/reports")

def read_json(name, default):
    path = reports / name
    if path.exists():
        return json.loads(path.read_text())
    return default

lint = read_json("lint_results.json", {})
sim = read_json("simulation_results.json", {})
impact = read_json("changed_rtl_impact.json", {})
verilator = read_json("verilator_lint_results.json", {})
icarus = read_json("icarus_sim_results.json", {})
yosys = read_json("yosys_check_results.json", {})

summary = {
    "lint_status": lint.get("status", "unknown"),
    "lint_failures": lint.get("failures", 0),
    "simulation_status": sim.get("status", "unknown"),
    "simulation_tests": sim.get("tests_run", 0),
    "simulation_passed": sim.get("tests_passed", 0),
    "impacted_tests": impact.get("impact_count", 0),
    "verilator_status": verilator.get("status", "unknown"),
    "icarus_status": icarus.get("status", "unknown"),
    "yosys_status": yosys.get("status", "unknown"),
}

lines = [
    "# CAD Flow Dashboard",
    "",
    "```json",
    json.dumps(summary, indent=2),
    "```",
    "",
    "| Area | Status / Value |",
    "|---|---|",
    f"| RTL lint | {summary['lint_status']} ({summary['lint_failures']} failures) |",
    f"| Mock simulation | {summary['simulation_status']} ({summary['simulation_passed']}/{summary['simulation_tests']} passing) |",
    f"| Changed-file impact | {summary['impacted_tests']} impacted tests |",
    f"| Verilator lint | {summary['verilator_status']} |",
    f"| Icarus simulation | {summary['icarus_status']} |",
    f"| Yosys check | {summary['yosys_status']} |",
    "",
    "## What this means",
    "",
    "This dashboard gives a CI-review summary for RTL lint-style checks, simulation regression, changed-file impact analysis, and synthesis-smoke checks.",
    "",
    "## Claims boundary",
    "",
    "This is an educational CAD-flow dashboard. It does not claim production EDA dashboarding or signoff ownership."
]

(reports / "cad_flow_dashboard.md").write_text("\n".join(lines) + "\n")
print(json.dumps(summary, indent=2))
