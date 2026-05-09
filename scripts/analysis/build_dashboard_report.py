from pathlib import Path
import json

comparison = json.loads(Path("reports/analysis/workload_comparison.json").read_text())
classification = json.loads(Path("reports/analysis/workload_classification.json").read_text())
sweep = json.loads(Path("reports/analysis/memory_sweep.json").read_text())

d = comparison["derived_metrics"]
s = sweep["comparison_1_to_4"]

md = f"""# AccelSim-Lite Performance Dashboard

## Summary

AccelSim-Lite is a deterministic workload-level accelerator simulator for analyzing throughput, latency, and bottleneck shifts under constrained compute and memory resources.

## Workload comparison

| Workload | Throughput | Avg Latency | Bottleneck | Classification |
|---|---:|---:|---|---|
"""

class_map = classification["summary"]

for r in comparison["workloads"]:
    md += f"| {r['workload']} | {r['throughput']:.4f} | {r['average_latency']:.4f} | {r['top_bottleneck']} | {class_map[r['workload']]} |\n"

md += f"""

## Derived metrics

- Throughput range: `{d['throughput_range_ops_per_cycle'][0]}–{d['throughput_range_ops_per_cycle'][1]} ops/cycle`
- Latency range: `{d['latency_range_cycles'][0]}–{d['latency_range_cycles'][1]} cycles`
- Memory-heavy throughput drop vs compute-heavy: `{d['memory_vs_compute_throughput_drop_percent']}%`
- Memory-heavy latency increase vs compute-heavy: `{d['memory_vs_compute_latency_increase_x']}x`

## What-if memory sweep

Memory ports `1 → 4` on `memory_heavy`:

- Latency improvement: `{s['latency_improvement_percent']}%`
- Throughput improvement: `{s['throughput_improvement_percent']}%`
- Bottleneck shift: `{s['bottleneck_shift']}`
- NoMemoryPort stall reduction: `{s['memory_port_stall_reduction']}`

## Charts

![Throughput by Workload](throughput_by_workload.png)

![Latency by Workload](latency_by_workload.png)

![Memory Sweep Latency](memory_sweep_latency.png)

![Memory Sweep Throughput](memory_sweep_throughput.png)

![Memory Sweep Stalls](memory_sweep_stalls.png)

## Interpretation

The simulator shows that memory bandwidth scaling improves performance until the memory bottleneck is removed. After that, dependency stalls dominate. This mirrors real performance engineering: optimization shifts the bottleneck rather than eliminating all bottlenecks.
"""

Path("reports/analysis/dashboard.md").write_text(md)
print("Generated reports/analysis/dashboard.md")
