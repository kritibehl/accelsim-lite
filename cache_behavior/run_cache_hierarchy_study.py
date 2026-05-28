#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT_JSON = ROOT / "cache_hierarchy_results.json"
OUT_MD = ROOT / "cache_hierarchy_report.md"
OUT_SVG = ROOT / "cache_latency_chart.svg"

profiles = [
    {
        "name": "hot_working_set",
        "description": "Small repeated working set with high L1 locality.",
        "l1_hit_rate": 0.86,
        "l2_hit_rate": 0.10,
        "memory_rate": 0.04
    },
    {
        "name": "mixed_locality",
        "description": "Moderate locality with partial L2 reuse.",
        "l1_hit_rate": 0.55,
        "l2_hit_rate": 0.30,
        "memory_rate": 0.15
    },
    {
        "name": "streaming_access",
        "description": "Large streaming access pattern with reduced cache reuse.",
        "l1_hit_rate": 0.22,
        "l2_hit_rate": 0.28,
        "memory_rate": 0.50
    },
    {
        "name": "pointer_chasing",
        "description": "Pointer-heavy pattern with poor locality and high memory access pressure.",
        "l1_hit_rate": 0.12,
        "l2_hit_rate": 0.18,
        "memory_rate": 0.70
    }
]

latencies = {
    "l1_cycles": 4,
    "l2_cycles": 12,
    "memory_cycles": 120
}

results = []
for p in profiles:
    avg_latency = (
        p["l1_hit_rate"] * latencies["l1_cycles"]
        + p["l2_hit_rate"] * latencies["l2_cycles"]
        + p["memory_rate"] * latencies["memory_cycles"]
    )
    slowdown = avg_latency / latencies["l1_cycles"]
    results.append({
        **p,
        "l1_cycles": latencies["l1_cycles"],
        "l2_cycles": latencies["l2_cycles"],
        "memory_cycles": latencies["memory_cycles"],
        "average_access_latency_cycles": round(avg_latency, 3),
        "slowdown_vs_all_l1_hits": round(slowdown, 3)
    })

payload = {
    "scope": "simulated cache hierarchy study; not hardware counter measurement",
    "latency_model": latencies,
    "results": results
}
OUT_JSON.write_text(json.dumps(payload, indent=2))

lines = [
    "# Cache Hierarchy Behavior Study",
    "",
    "> Scope: simulated cache hierarchy behavior. This is not hardware-counter measurement, CPU microarchitecture profiling, or production performance benchmarking.",
    "",
    "## Latency model",
    "",
    "| Access level | Simulated latency |",
    "|---|---:|",
    f"| L1 hit | {latencies['l1_cycles']} cycles |",
    f"| L2 hit | {latencies['l2_cycles']} cycles |",
    f"| Memory access | {latencies['memory_cycles']} cycles |",
    "",
    "## Results",
    "",
    "| Profile | L1 hit | L2 hit | Memory access | Avg latency cycles | Slowdown vs all-L1 |",
    "|---|---:|---:|---:|---:|---:|"
]

for r in results:
    lines.append(
        f"| {r['name']} | {r['l1_hit_rate']:.0%} | {r['l2_hit_rate']:.0%} | "
        f"{r['memory_rate']:.0%} | {r['average_access_latency_cycles']} | "
        f"{r['slowdown_vs_all_l1_hits']}x |"
    )

lines += [
    "",
    "## What this means",
    "",
    "- Hot working sets stay closer to L1 behavior and keep average access latency low.",
    "- Mixed-locality workloads become more sensitive to L2 and memory behavior.",
    "- Streaming and pointer-chasing patterns experience much higher average latency because more accesses fall through to memory.",
    "- This demonstrates cache hierarchy reasoning, memory-locality tradeoffs, and latency impact analysis for systems-performance discussion.",
    "",
    "## Claims boundary",
    "",
    "This is a simulated cache hierarchy study. It does not claim real hardware cache measurement, perf-counter analysis, or CPU microarchitecture benchmarking."
]

OUT_MD.write_text("\n".join(lines) + "\n")

# Simple standalone SVG bar chart.
max_latency = max(r["average_access_latency_cycles"] for r in results)
bar_height = 28
gap = 18
width = 820
height = 120 + len(results) * (bar_height + gap)

svg = [
    f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
    '<rect width="100%" height="100%" fill="#ffffff"/>',
    '<text x="30" y="40" font-family="Arial" font-size="24" font-weight="700">Simulated Cache Hierarchy Latency</text>',
    '<text x="30" y="68" font-family="Arial" font-size="13" fill="#555">Average access latency by locality profile; simulated cycles, not hardware counters.</text>'
]

y = 100
for r in results:
    bar_width = int((r["average_access_latency_cycles"] / max_latency) * 520)
    svg.append(f'<text x="30" y="{y + 20}" font-family="Arial" font-size="14">{r["name"]}</text>')
    svg.append(f'<rect x="230" y="{y}" width="{bar_width}" height="{bar_height}" rx="6" fill="#2563eb"/>')
    svg.append(f'<text x="{240 + bar_width}" y="{y + 20}" font-family="Arial" font-size="14" fill="#111">{r["average_access_latency_cycles"]} cycles</text>')
    y += bar_height + gap

svg.append('</svg>')
OUT_SVG.write_text("\n".join(svg) + "\n")

print(f"Generated {OUT_JSON}")
print(f"Generated {OUT_MD}")
print(f"Generated {OUT_SVG}")
