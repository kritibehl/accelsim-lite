import json
from pathlib import Path

comparison = json.loads(
    Path("reports/analysis/workload_comparison.json").read_text()
)

derived = comparison["derived_metrics"]

print("=== Comparison Summary ===")
print(f"Throughput range: {derived['throughput_range_ops_per_cycle']}")
print(f"Latency range: {derived['latency_range_cycles']}")
