import json
from pathlib import Path
import matplotlib.pyplot as plt

data = json.loads(Path("reports/analysis/memory_sweep.json").read_text())
rows = data["sweep"]

ports = [r["memory_ports"] for r in rows]
latency = [r["average_latency"] for r in rows]
throughput = [r["throughput"] for r in rows]
memory_stalls = [r["stall_counts"].get("NoMemoryPort", 0) for r in rows]

out = Path("reports/analysis")
out.mkdir(parents=True, exist_ok=True)

plt.figure(figsize=(8, 5))
plt.plot(ports, latency, marker="o")
plt.xlabel("Memory Ports")
plt.ylabel("Average Latency (cycles)")
plt.title("Latency vs Memory Ports")
plt.tight_layout()
plt.savefig(out / "memory_sweep_latency.png")
plt.close()

plt.figure(figsize=(8, 5))
plt.plot(ports, throughput, marker="o")
plt.xlabel("Memory Ports")
plt.ylabel("Throughput (ops/cycle)")
plt.title("Throughput vs Memory Ports")
plt.tight_layout()
plt.savefig(out / "memory_sweep_throughput.png")
plt.close()

plt.figure(figsize=(8, 5))
plt.plot(ports, memory_stalls, marker="o")
plt.xlabel("Memory Ports")
plt.ylabel("NoMemoryPort Stalls")
plt.title("Memory Stalls vs Memory Ports")
plt.tight_layout()
plt.savefig(out / "memory_sweep_stalls.png")
plt.close()

print("Generated memory sweep charts.")
