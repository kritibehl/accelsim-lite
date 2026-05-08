import json
from pathlib import Path

import matplotlib.pyplot as plt

data = json.loads(Path("reports/analysis/workload_comparison.json").read_text())
rows = data["workloads"]

names = [r["workload"] for r in rows]
throughput = [r["throughput"] for r in rows]
latency = [r["average_latency"] for r in rows]

out_dir = Path("reports/analysis")
out_dir.mkdir(parents=True, exist_ok=True)

plt.figure(figsize=(9, 5))
plt.bar(names, throughput)
plt.ylabel("Throughput (ops/cycle)")
plt.title("Throughput by Workload")
plt.xticks(rotation=20, ha="right")
plt.tight_layout()
plt.savefig(out_dir / "throughput_by_workload.png")
plt.close()

plt.figure(figsize=(9, 5))
plt.bar(names, latency)
plt.ylabel("Average Latency (cycles)")
plt.title("Latency by Workload")
plt.xticks(rotation=20, ha="right")
plt.tight_layout()
plt.savefig(out_dir / "latency_by_workload.png")
plt.close()

print("Generated:")
print(out_dir / "throughput_by_workload.png")
print(out_dir / "latency_by_workload.png")
