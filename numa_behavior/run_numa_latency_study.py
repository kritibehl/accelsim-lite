#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT_JSON = ROOT / "numa_latency_results.json"
OUT_MD = ROOT / "numa_latency_report.md"

profiles = [
    {"name": "local_memory_access", "local_rate": 0.92, "remote_rate": 0.08},
    {"name": "balanced_cross_socket_access", "local_rate": 0.60, "remote_rate": 0.40},
    {"name": "remote_heavy_access", "local_rate": 0.30, "remote_rate": 0.70}
]

latency_model = {
    "local_memory_cycles": 80,
    "remote_memory_cycles": 155
}

results = []
for p in profiles:
    avg = (
        p["local_rate"] * latency_model["local_memory_cycles"]
        + p["remote_rate"] * latency_model["remote_memory_cycles"]
    )
    delta_vs_local = avg - latency_model["local_memory_cycles"]
    results.append({
        **p,
        "average_latency_cycles": round(avg, 3),
        "latency_delta_vs_all_local_cycles": round(delta_vs_local, 3),
        "slowdown_vs_all_local": round(avg / latency_model["local_memory_cycles"], 3)
    })

payload = {
    "scope": "simulated NUMA locality study; not hardware NUMA measurement",
    "latency_model": latency_model,
    "results": results
}
OUT_JSON.write_text(json.dumps(payload, indent=2))

lines = [
    "# Simulated NUMA Local vs Remote Memory Study",
    "",
    "> Scope: simulated NUMA-locality study. This is not hardware NUMA profiling or production performance benchmarking.",
    "",
    "## Latency model",
    "",
    "| Access type | Simulated latency |",
    "|---|---:|",
    f"| Local memory | {latency_model['local_memory_cycles']} cycles |",
    f"| Remote memory | {latency_model['remote_memory_cycles']} cycles |",
    "",
    "## Results",
    "",
    "| Profile | Local access | Remote access | Avg latency | Delta vs all-local | Slowdown |",
    "|---|---:|---:|---:|---:|---:|"
]

for r in results:
    lines.append(
        f"| {r['name']} | {r['local_rate']:.0%} | {r['remote_rate']:.0%} | "
        f"{r['average_latency_cycles']} cycles | "
        f"{r['latency_delta_vs_all_local_cycles']} cycles | "
        f"{r['slowdown_vs_all_local']}x |"
    )

lines += [
    "",
    "## What this means",
    "",
    "- Local memory access keeps latency closer to the baseline local-memory model.",
    "- Cross-socket or remote-heavy access increases average memory latency.",
    "- This demonstrates locality-aware systems reasoning, memory-placement tradeoffs, and latency-impact analysis.",
    "",
    "## Claims boundary",
    "",
    "This is a simulated NUMA-locality study. It does not claim real NUMA hardware measurement, OS scheduler implementation, or CPU performance-counter profiling."
]

OUT_MD.write_text("\n".join(lines) + "\n")

print(f"Generated {OUT_JSON}")
print(f"Generated {OUT_MD}")
