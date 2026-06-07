#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_JSON = Path("engineering_performance_center/performance_center_summary.json")
OUT_MD = Path("engineering_performance_center/performance_center_report.md")

runtime_advisor = json.loads((ROOT / "runtime_advisor/runtime_advisor_report.json").read_text())
build_health = json.loads((ROOT / "build_analytics/regression_trends.json").read_text())
regression = json.loads((ROOT / "regression_explorer/regression_summary.json").read_text())
optimization = json.loads((ROOT / "optimization_explorer/optimization_comparison.json").read_text())

advisor_first = runtime_advisor["advisor_outputs"][0]
best_speedup = max(optimization["experiments"], key=lambda e: e["speedup_x"])

summary = {
    "biggest_bottleneck": advisor_first["bottleneck"],
    "bottleneck_workload": advisor_first["workload"],
    "recommendation": advisor_first["recommendation"],
    "speedup_opportunity": f"{best_speedup['speedup_x']}x",
    "speedup_source": best_speedup["name"],
    "failed_builds": build_health["failed_builds"],
    "flaky_tests": build_health["flaky_tests"],
    "regression_detected": build_health["regression_detected"],
    "top_root_cause": build_health["root_cause"],
    "release_decision": regression["decision"],
    "release_root_cause": regression["root_cause"],
    "engineering_health": "needs_review" if regression["decision"] == "block_release" or build_health["failed_builds"] > 3 else "healthy"
}

OUT_JSON.write_text(json.dumps(summary, indent=2))

lines = [
    "# Engineering Performance Center",
    "",
    "> Scope: executive-readable summary over AccelSim-Lite performance, build-health, regression, and optimization signals.",
    "",
    "## Summary",
    "",
    "```json",
    json.dumps(summary, indent=2),
    "```",
    "",
    "## What this center shows",
    "",
    "| Area | Signal |",
    "|---|---|",
    f"| Biggest bottleneck | {summary['biggest_bottleneck']} on `{summary['bottleneck_workload']}` |",
    f"| Recommendation | {summary['recommendation']} |",
    f"| Speedup opportunity | {summary['speedup_opportunity']} from `{summary['speedup_source']}` |",
    f"| Failed builds | {summary['failed_builds']} |",
    f"| Flaky tests | {summary['flaky_tests']} |",
    f"| Build root cause | {summary['top_root_cause']} |",
    f"| Release decision | {summary['release_decision']} |",
    f"| Engineering health | {summary['engineering_health']} |",
    "",
    "## What this means",
    "",
    "This center turns low-level runtime/performance/build data into a concise engineering-health view: bottleneck, recommendation, speedup opportunity, build health, release risk, and root cause.",
    "",
    "## Claims boundary",
    "",
    "This is an educational engineering-performance summary over AccelSim-Lite simulator-derived reports. It does not claim production performance ownership or internal build/release infrastructure."
]

OUT_MD.write_text("\n".join(lines) + "\n")

print(json.dumps(summary, indent=2))
