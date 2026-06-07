#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
A = json.loads((ROOT / "release_a.json").read_text())
B = json.loads((ROOT / "release_b.json").read_text())

def pct_change(a, b):
    return round(100 * (b - a) / a, 2)

metrics = []

def add_metric(name, key, direction):
    a = A["benchmarks"][key]
    b = B["benchmarks"][key]
    change = pct_change(a, b)

    if direction == "lower_is_better":
        regression = change > 20
        improvement = change < -10
    else:
        regression = change < -10
        improvement = change > 10

    metrics.append({
        "metric": name,
        "release_a": a,
        "release_b": b,
        "change_percent": change,
        "direction": direction,
        "classification": "regression" if regression else "improvement" if improvement else "neutral"
    })

add_metric("metadata_lookup_p95_ns", "metadata_lookup_p95_ns", "lower_is_better")
add_metric("metadata_lookup_throughput_ops_sec", "metadata_lookup_throughput_ops_sec", "higher_is_better")
add_metric("cache_locality_latency_cycles", "cache_locality_latency_cycles", "lower_is_better")
add_metric("threading_queue_throughput_ops_sec", "threading_queue_throughput_ops_sec", "higher_is_better")

failed_build_delta = B["build_health"]["failed_builds"] - A["build_health"]["failed_builds"]
flaky_test_delta = B["build_health"]["flaky_tests"] - A["build_health"]["flaky_tests"]

regressions = [m for m in metrics if m["classification"] == "regression"]

root_cause = "metadata lookup path and cache locality regression"
if any(m["metric"] == "cache_locality_latency_cycles" and m["classification"] == "regression" for m in regressions):
    root_cause = "cache locality / memory layout regression"
if any(m["metric"] == "metadata_lookup_p95_ns" and m["classification"] == "regression" for m in regressions):
    root_cause = "metadata lookup path regression"

decision = "block_release" if regressions or failed_build_delta >= 3 else "pass_release"

summary = {
    "release_a": A["release"],
    "release_b": B["release"],
    "regressions": len(regressions),
    "failed_build_delta": failed_build_delta,
    "flaky_test_delta": flaky_test_delta,
    "root_cause": root_cause,
    "decision": decision,
    "metrics": metrics
}

(ROOT / "regression_summary.json").write_text(json.dumps(summary, indent=2))

lines = [
    "# Regression Explorer",
    "",
    "> Scope: release-to-release comparison for AccelSim-Lite benchmark and build-health signals.",
    "",
    "## Executive summary",
    "",
    "```json",
    json.dumps({
        "release_a": summary["release_a"],
        "release_b": summary["release_b"],
        "regressions": summary["regressions"],
        "failed_build_delta": summary["failed_build_delta"],
        "flaky_test_delta": summary["flaky_test_delta"],
        "root_cause": summary["root_cause"],
        "decision": summary["decision"]
    }, indent=2),
    "```",
    "",
    "## Benchmark comparison",
    "",
    "| Metric | Release A | Release B | Change | Classification |",
    "|---|---:|---:|---:|---|"
]

for m in metrics:
    lines.append(
        f"| {m['metric']} | {m['release_a']} | {m['release_b']} | "
        f"{m['change_percent']}% | {m['classification']} |"
    )

lines += [
    "",
    "## Build-health comparison",
    "",
    f"- Failed builds: {A['build_health']['failed_builds']} → {B['build_health']['failed_builds']} ({failed_build_delta:+d})",
    f"- Flaky tests: {A['build_health']['flaky_tests']} → {B['build_health']['flaky_tests']} ({flaky_test_delta:+d})",
    "",
    "## Decision",
    "",
    f"`{decision}`",
    "",
    "## What this means",
    "",
    "The Regression Explorer turns performance/build data into a release decision by comparing benchmark drift, build-health movement, root-cause signals, and release risk.",
    "",
    "## Claims boundary",
    "",
    "This is an educational release-regression workflow over simulator-derived benchmark/build-health data. It does not claim production release ownership."
]

(ROOT / "regression_explorer_report.md").write_text("\n".join(lines) + "\n")

print(json.dumps({
    "release_a": summary["release_a"],
    "release_b": summary["release_b"],
    "root_cause": summary["root_cause"],
    "decision": summary["decision"]
}, indent=2))
