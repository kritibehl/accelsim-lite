#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
baseline = json.loads((ROOT / "baseline.json").read_text())["benchmarks"]
candidate = json.loads((ROOT / "candidate.json").read_text())["benchmarks"]

baseline_by_name = {b["name"]: b for b in baseline}
rows = []
failures = []

P95_REGRESSION_THRESHOLD = 25.0
THROUGHPUT_DROP_THRESHOLD = 10.0

for c in candidate:
    name = c["name"]
    b = baseline_by_name[name]

    p95_change = 100.0 * (c["p95_ns"] - b["p95_ns"]) / b["p95_ns"]
    throughput_change = 100.0 * (c["throughput_ops_sec"] - b["throughput_ops_sec"]) / b["throughput_ops_sec"]

    decision = "pass"
    reasons = []

    if p95_change > P95_REGRESSION_THRESHOLD:
        decision = "fail"
        reasons.append(f"p95 latency regression {p95_change:.2f}% > {P95_REGRESSION_THRESHOLD}%")

    if throughput_change < -THROUGHPUT_DROP_THRESHOLD:
        decision = "fail"
        reasons.append(f"throughput drop {throughput_change:.2f}% < -{THROUGHPUT_DROP_THRESHOLD}%")

    row = {
        "benchmark": name,
        "baseline_p95_ns": b["p95_ns"],
        "candidate_p95_ns": c["p95_ns"],
        "p95_regression_percent": round(p95_change, 2),
        "baseline_throughput_ops_sec": b["throughput_ops_sec"],
        "candidate_throughput_ops_sec": c["throughput_ops_sec"],
        "throughput_change_percent": round(throughput_change, 2),
        "decision": decision,
        "reasons": reasons
    }
    rows.append(row)

    if decision == "fail":
        failures.append(row)

summary = {
    "scope": "C++ performance regression gate over synthetic benchmark summaries",
    "p95_regression_threshold_percent": P95_REGRESSION_THRESHOLD,
    "throughput_drop_threshold_percent": THROUGHPUT_DROP_THRESHOLD,
    "total_benchmarks": len(rows),
    "failed_benchmarks": len(failures),
    "decision": "fail" if failures else "pass",
    "results": rows
}

(ROOT / "perf_summary.json").write_text(json.dumps(summary, indent=2))

lines = [
    "# Performance Regression Gate Report",
    "",
    "> Scope: baseline-vs-candidate benchmark comparison for C++ runtime/performance experiments. This is not production performance certification.",
    "",
    "| Benchmark | Baseline p95 ns | Candidate p95 ns | p95 change | Throughput change | Decision |",
    "|---|---:|---:|---:|---:|---|"
]

for r in rows:
    lines.append(
        f"| {r['benchmark']} | {r['baseline_p95_ns']} | {r['candidate_p95_ns']} | "
        f"{r['p95_regression_percent']}% | {r['throughput_change_percent']}% | {r['decision'].upper()} |"
    )

lines += [
    "",
    "## Failure diagnostics",
    ""
]

if failures:
    for f in failures:
        lines.append(f"- `{f['benchmark']}`: " + "; ".join(f["reasons"]))
else:
    lines.append("- No performance regressions exceeded configured thresholds.")

lines += [
    "",
    "## Final decision",
    "",
    f"`{summary['decision'].upper()}`",
    "",
    "## Claims boundary",
    "",
    "This gate compares checked-in benchmark summaries and demonstrates regression-gating methodology. It does not claim production-scale benchmarking."
]

(ROOT / "perf_report.md").write_text("\n".join(lines) + "\n")

print(json.dumps({
    "total_benchmarks": summary["total_benchmarks"],
    "failed_benchmarks": summary["failed_benchmarks"],
    "decision": summary["decision"]
}, indent=2))
