import json
import re
import subprocess
from pathlib import Path

BIN = "./build/accelsim-lite"
WORKLOAD = "workloads/memory_heavy.csv"
PORTS = [1, 2, 4]
OUT_DIR = Path("reports/analysis")
OUT_DIR.mkdir(parents=True, exist_ok=True)

def run(ports):
    out = subprocess.check_output(
        [BIN, "run", WORKLOAD, "--memory-ports", str(ports)],
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
        "memory_ports": ports,
        "total_cycles": int(grab(r"total_cycles=(\d+)")),
        "throughput": float(grab(r"throughput=(\d+\.\d+)")),
        "average_latency": float(grab(r"average_latency=(\d+\.\d+)")),
        "top_bottleneck": grab(r"top_bottleneck=(\w+)"),
        "stall_counts": stalls,
    }

rows = [run(p) for p in PORTS]
base = rows[0]
opt = rows[-1]

result = {
    "workload": "memory_heavy",
    "sweep": rows,
    "comparison_1_to_4": {
        "latency_improvement_percent": round((base["average_latency"] - opt["average_latency"]) / base["average_latency"] * 100, 2),
        "throughput_improvement_percent": round((opt["throughput"] - base["throughput"]) / base["throughput"] * 100, 2),
        "bottleneck_shift": f"{base['top_bottleneck']} -> {opt['top_bottleneck']}",
        "memory_port_stall_reduction": base["stall_counts"].get("NoMemoryPort", 0) - opt["stall_counts"].get("NoMemoryPort", 0),
    },
}

Path("reports/analysis/memory_sweep.json").write_text(json.dumps(result, indent=2))

md = ["# Memory-Port What-If Sweep\n"]
md.append("| Memory Ports | Throughput | Avg Latency | Top Bottleneck | NoMemoryPort Stalls |")
md.append("|---:|---:|---:|---|---:|")
for r in rows:
    md.append(
        f"| {r['memory_ports']} | {r['throughput']:.4f} | {r['average_latency']:.4f} | "
        f"{r['top_bottleneck']} | {r['stall_counts'].get('NoMemoryPort', 0)} |"
    )

cmp = result["comparison_1_to_4"]
md.append("\n## 1 → 4 memory ports\n")
md.append(f"- Latency improvement: `{cmp['latency_improvement_percent']}%`")
md.append(f"- Throughput improvement: `{cmp['throughput_improvement_percent']}%`")
md.append(f"- Bottleneck shift: `{cmp['bottleneck_shift']}`")
md.append(f"- NoMemoryPort stall reduction: `{cmp['memory_port_stall_reduction']}`")

Path("reports/analysis/memory_sweep.md").write_text("\n".join(md) + "\n")

print(json.dumps(result, indent=2))
