import subprocess
import re
import json

BIN = "./build/accelsim-lite"
WORKLOAD = "workloads/memory_heavy.csv"

def run(mem_ports):
    out = subprocess.check_output(
        [BIN, "run", WORKLOAD, "--memory-ports", str(mem_ports)],
        text=True
    )
    latency = float(re.search(r"average_latency=(\d+\.\d+)", out).group(1))
    throughput = float(re.search(r"throughput=(\d+\.\d+)", out).group(1))
    bottleneck = re.search(r"top_bottleneck=(\w+)", out).group(1)
    return latency, throughput, bottleneck

lat1, thr1, bott1 = run(1)
lat2, thr2, bott2 = run(4)

lat_improvement = (lat1 - lat2) / lat1 * 100 if lat1 else 0.0
thr_improvement = (thr2 - thr1) / thr1 * 100 if thr1 else 0.0

result = {
    "workload": "memory_heavy",
    "baseline": {
        "memory_ports": 1,
        "latency": round(lat1, 4),
        "throughput": round(thr1, 4),
        "top_bottleneck": bott1
    },
    "optimized": {
        "memory_ports": 4,
        "latency": round(lat2, 4),
        "throughput": round(thr2, 4),
        "top_bottleneck": bott2
    },
    "latency_improvement_percent": round(lat_improvement, 2),
    "throughput_improvement_percent": round(thr_improvement, 2)
}

print(json.dumps(result, indent=2))
