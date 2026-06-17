#!/usr/bin/env python3
import json
from pathlib import Path

lint = json.loads(Path("cad_flow/reports/lint_results.json").read_text())
sim = json.loads(Path("cad_flow/reports/simulation_results.json").read_text())
impact = json.loads(Path("cad_flow/reports/changed_rtl_impact.json").read_text())

summary = {
    "lint_status": lint["status"],
    "lint_failures": lint["failures"],
    "simulation_status": sim["status"],
    "tests_run": sim["tests_run"],
    "tests_passed": sim["tests_passed"],
    "impacted_tests": impact["impact_count"],
    "cad_flow_status": "pass" if lint["status"] == "pass" and sim["status"] == "pass" else "fail"
}

Path("cad_flow/reports/cad_flow_summary.json").write_text(json.dumps(summary, indent=2))

lines = [
    "# CAD Flow Regression Summary",
    "",
    "```json",
    json.dumps(summary, indent=2),
    "```",
    "",
    "## What this demonstrates",
    "",
    "- RTL lint automation",
    "- simulation regression summarization",
    "- changed-file impact analysis",
    "- CI/reporting-friendly CAD workflow outputs",
    "",
    "## Claims boundary",
    "",
    "This is a portfolio-safe CAD-flow automation module. It does not claim production EDA signoff, synthesis ownership, or Google internal CAD infrastructure experience."
]

Path("cad_flow/reports/regression_summary.md").write_text("\n".join(lines) + "\n")
print(json.dumps(summary, indent=2))
