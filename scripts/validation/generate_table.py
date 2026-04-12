import subprocess
import re

workloads = ["compute_heavy", "memory_heavy", "mixed", "queue_pressure"]

rows = []

for w in workloads:
    out = subprocess.check_output(
        ["./build/accelsim-lite", "run", f"workloads/{w}.csv"],
        text=True
    )

    throughput = float(re.search(r"throughput=(\d+\.\d+)", out).group(1))
    latency = float(re.search(r"average_latency=(\d+\.\d+)", out).group(1))
    bottleneck = re.search(r"top_bottleneck=(\w+)", out).group(1)

    rows.append((w, throughput, latency, bottleneck))

print("\n=== VALIDATION TABLE ===\n")
print(f"{'Workload':<15} {'Throughput':<12} {'Latency':<10} {'Bottleneck'}")
print("-" * 55)

for r in rows:
    print(f"{r[0]:<15} {r[1]:<12.4f} {r[2]:<10.2f} {r[3]}")
