#!/usr/bin/env python3
import json
from pathlib import Path

OUT_JSON = Path("optimization_explorer/optimization_comparison.json")
OUT_MD = Path("optimization_explorer/optimization_explorer_report.md")

experiments = [
    {
        "name": "metadata_lookup_path",
        "baseline_latency_ns": 620,
        "optimized_latency_ns": 410,
        "baseline_throughput": 2100000,
        "optimized_throughput": 2400000,
        "optimization": "restore faster metadata lookup path"
    },
    {
        "name": "cache_locality_layout",
        "baseline_latency_cycles": 86.64,
        "optimized_latency_cycles": 23.80,
        "baseline_throughput": 1.0,
        "optimized_throughput": 3.64,
        "optimization": "move pointer-heavy traversal toward mixed-locality layout"
    },
    {
        "name": "threading_queue_partitioning",
        "baseline_latency_ns": 2200,
        "optimized_latency_ns": 1800,
        "baseline_throughput": 410000,
        "optimized_throughput": 445000,
        "optimization": "reduce shared queue contention"
    }
]

for e in experiments:
    if "baseline_latency_ns" in e:
        base = e["baseline_latency_ns"]
        opt = e["optimized_latency_ns"]
    else:
        base = e["baseline_latency_cycles"]
        opt = e["optimized_latency_cycles"]

    e["latency_change_percent"] = round(100 * (opt - base) / base, 2)
    e["speedup_x"] = round(base / opt, 3)
    e["throughput_change_percent"] = round(
        100 * (e["optimized_throughput"] - e["baseline_throughput"]) / e["baseline_throughput"],
        2
    )
    e["decision"] = "improved" if opt < base else "regressed"

payload = {
    "scope": "baseline-vs-optimized comparison layer over AccelSim-Lite studies",
    "experiments": experiments
}
OUT_JSON.write_text(json.dumps(payload, indent=2))

lines = [
    "# Optimization Explorer",
    "",
    "> Scope: readable baseline-vs-optimized comparison over AccelSim-Lite benchmark studies.",
    "",
    "| Experiment | Optimization | Latency change | Speedup | Throughput change | Decision |",
    "|---|---|---:|---:|---:|---|"
]

for e in experiments:
    lines.append(
        f"| {e['name']} | {e['optimization']} | {e['latency_change_percent']}% | "
        f"{e['speedup_x']}x | {e['throughput_change_percent']}% | {e['decision']} |"
    )

lines += [
    "",
    "## What this proves",
    "",
    "This explorer turns performance experiments into a clear before/after story: baseline, optimized, speedup, throughput movement, and regression/improvement decision.",
    "",
    "## Claims boundary",
    "",
    "This is an educational optimization-comparison layer over simulator-derived reports. It does not claim production autotuning or hardware-level optimization."
]

OUT_MD.write_text("\n".join(lines) + "\n")

print(f"Generated {OUT_JSON}")
print(f"Generated {OUT_MD}")
