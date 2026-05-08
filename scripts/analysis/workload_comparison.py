import json
import re
import subprocess
from pathlib import Path

BIN = "./build/accelsim-lite"
WORKLOADS = [
    "compute_heavy",
    "memory_heavy",
    "mixed",
    "queue_pressure",
]

OUT_DIR = Path("reports/analysis")
OUT_DIR.mkdir(parents=True, exist_ok=True)

def run_workload(name):
    out = subprocess.check_output(
        [BIN, "run", f"workloads/{name}.csv"],
        text=True,
    )

    def grab(pattern):
        m = re.search(pattern, out)
        return m.group(1) if m else None

    stalls = {}
    in_stalls = False
    for line in out.splitlines():
        if line.strip() == "[stall_counts]":
            in_stalls = True
            continue
        if in_stalls and line.startswith("["):
            break
        if in_stalls and "=" in line:
            k, v = line.split("=", 1)
            stalls[k.strip()] = int(v.strip())

    return {
        "workload": name,
        "total_cycles": int(grab(r"total_cycles=(\d+)")),
        "completed_ops": int(grab(r"completed_ops=(\d+)")),
        "throughput": float(grab(r"throughput=(\d+\.\d+)")),
        "average_latency": float(grab(r"average_latency=(\d+\.\d+)")),
        "top_bottleneck": grab(r"top_bottleneck=(\w+)"),
        "stall_counts": stalls,
    }

rows = [run_workload(w) for w in WORKLOADS]

compute = next(r for r in rows if r["workload"] == "compute_heavy")
memory = next(r for r in rows if r["workload"] == "memory_heavy")

throughput_drop_pct = (compute["throughput"] - memory["throughput"]) / compute["throughput"] * 100
latency_increase_x = memory["average_latency"] / compute["average_latency"]

report = {
    "workloads": rows,
    "derived_metrics": {
        "throughput_range_ops_per_cycle": [
            round(min(r["throughput"] for r in rows), 4),
            round(max(r["throughput"] for r in rows), 4),
        ],
        "latency_range_cycles": [
            round(min(r["average_latency"] for r in rows), 4),
            round(max(r["average_latency"] for r in rows), 4),
        ],
        "memory_vs_compute_throughput_drop_percent": round(throughput_drop_pct, 2),
        "memory_vs_compute_latency_increase_x": round(latency_increase_x, 2),
    },
}

json_path = OUT_DIR / "workload_comparison.json"
md_path = OUT_DIR / "workload_comparison.md"

json_path.write_text(json.dumps(report, indent=2))

lines = []
lines.append("# Workload Comparison Report\n")
lines.append("| Workload | Cycles | Ops | Throughput | Avg Latency | Top Bottleneck |")
lines.append("|---|---:|---:|---:|---:|---|")
for r in rows:
    lines.append(
        f"| {r['workload']} | {r['total_cycles']} | {r['completed_ops']} | "
        f"{r['throughput']:.4f} | {r['average_latency']:.4f} | {r['top_bottleneck']} |"
    )

lines.append("\n## Derived metrics\n")
lines.append(f"- Throughput range: `{report['derived_metrics']['throughput_range_ops_per_cycle'][0]}--{report['derived_metrics']['throughput_range_ops_per_cycle'][1]} ops/cycle`")
lines.append(f"- Latency range: `{report['derived_metrics']['latency_range_cycles'][0]}--{report['derived_metrics']['latency_range_cycles'][1]} cycles`")
lines.append(f"- Memory-heavy throughput drop vs compute-heavy: `{throughput_drop_pct:.2f}%`")
lines.append(f"- Memory-heavy latency increase vs compute-heavy: `{latency_increase_x:.2f}x`")

md_path.write_text("\n".join(lines) + "\n")

print(json.dumps(report, indent=2))
