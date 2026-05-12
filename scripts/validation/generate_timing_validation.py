import json
import subprocess
import re
from pathlib import Path

BIN = "./build/accelsim-lite"
OUT = Path("validation")
OUT.mkdir(parents=True, exist_ok=True)

WORKLOADS = ["compute_heavy", "memory_heavy", "mixed", "queue_pressure"]

BASELINES = {
    "compute_heavy": 18,
    "memory_heavy": 43,
    "mixed": 30,
    "queue_pressure": 25,
}

def parse_run(name):
    out = subprocess.check_output(
        [BIN, "run", f"workloads/{name}.csv"],
        text=True,
    )

    def grab(pattern, cast=str):
        m = re.search(pattern, out)
        if not m:
            raise ValueError(f"missing pattern {pattern} for {name}")
        return cast(m.group(1))

    stage_busy = {}
    stalls = {}

    section = None
    for line in out.splitlines():
        line = line.strip()
        if line == "[stage_busy_cycles]":
            section = "stage"
            continue
        if line == "[stall_counts]":
            section = "stall"
            continue
        if line.startswith("[") and line.endswith("]"):
            section = None
            continue

        if "=" in line and section in {"stage", "stall"}:
            k, v = line.split("=", 1)
            if section == "stage":
                stage_busy[k] = int(v)
            else:
                stalls[k] = int(v)

    total_cycles = grab(r"total_cycles=(\d+)", int)
    expected = BASELINES[name]
    drift = total_cycles - expected
    drift_pct = (drift / expected) * 100 if expected else 0.0

    return {
        "workload": name,
        "expected_cycles": expected,
        "observed_cycles": total_cycles,
        "cycle_drift": drift,
        "cycle_drift_percent": round(drift_pct, 2),
        "average_latency": grab(r"average_latency=(\d+\.\d+)", float),
        "throughput": grab(r"throughput=(\d+\.\d+)", float),
        "top_bottleneck": grab(r"top_bottleneck=(\w+)"),
        "pipeline_stage_busy_cycles": stage_busy,
        "stall_timing_cycles": stalls,
    }

results = [parse_run(w) for w in WORKLOADS]

report = {
    "scope": "simulator-cycle timing validation; not real hardware timing",
    "baseline_source": "known deterministic workload outputs from AccelSim-Lite validation suite",
    "timing_regressions": results,
    "summary": {
        "max_abs_cycle_drift": max(abs(r["cycle_drift"]) for r in results),
        "regression_detected": any(r["cycle_drift"] != 0 for r in results),
        "workload_count": len(results),
    },
}

(OUT / "timing_regression_report.json").write_text(json.dumps(report, indent=2))

md = ["# Clock Cycle Breakdown\n"]
md.append("> Scope: simulator-cycle timing validation, not hardware-cycle prediction.\n")
md.append("| Workload | Expected Cycles | Observed Cycles | Drift | Drift % | Avg Latency | Throughput | Top Bottleneck |")
md.append("|---|---:|---:|---:|---:|---:|---:|---|")

for r in results:
    md.append(
        f"| {r['workload']} | {r['expected_cycles']} | {r['observed_cycles']} | "
        f"{r['cycle_drift']} | {r['cycle_drift_percent']}% | "
        f"{r['average_latency']:.4f} | {r['throughput']:.4f} | {r['top_bottleneck']} |"
    )

md.append("\n## Pipeline-stage timing\n")
for r in results:
    md.append(f"### {r['workload']}\n")
    md.append("| Stage | Busy Cycles |")
    md.append("|---|---:|")
    for stage, cycles in r["pipeline_stage_busy_cycles"].items():
        md.append(f"| {stage} | {cycles} |")
    md.append("")

md.append("## Stall timing\n")
for r in results:
    md.append(f"### {r['workload']}\n")
    md.append("| Stall Reason | Cycles |")
    md.append("|---|---:|")
    for reason, cycles in r["stall_timing_cycles"].items():
        md.append(f"| {reason} | {cycles} |")
    md.append("")

md.append("## Interpretation\n")
md.append("- Cycle drift is measured against deterministic simulator baselines.")
md.append("- A non-zero drift indicates a timing regression in simulator behavior.")
md.append("- Stall timing helps identify whether latency is dominated by dependency, memory-port, compute, or queue pressure.")

(OUT / "clock_cycle_breakdown.md").write_text("\n".join(md) + "\n")

print(json.dumps(report, indent=2))
print("Generated validation/timing_regression_report.json")
print("Generated validation/clock_cycle_breakdown.md")
