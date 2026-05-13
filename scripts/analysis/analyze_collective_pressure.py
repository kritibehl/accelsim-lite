import json
from pathlib import Path

OUT = Path("analysis")
OUT.mkdir(parents=True, exist_ok=True)

# Scope:
# This is a workload-level communication-pressure model.
# It is NOT modeling real NCCL, EFA, PCIe, NVLink, or hardware collectives.

GROUPS = [1, 2, 4]

SCENARIOS = [
    {
        "name": "compute_heavy",
        "compute_cycles": 120,
        "transfer_cycles_per_group": 8,
        "base_queue_delay": 4,
    },
    {
        "name": "transfer_heavy_all_gather_style",
        "compute_cycles": 80,
        "transfer_cycles_per_group": 42,
        "base_queue_delay": 8,
    },
    {
        "name": "reduction_style_pressure",
        "compute_cycles": 96,
        "transfer_cycles_per_group": 28,
        "base_queue_delay": 6,
    },
]

def classify(compute_cycles, transfer_cycles, queue_delay):
    if transfer_cycles > compute_cycles * 0.75:
        return "communication-pressure-limited"
    if transfer_cycles > compute_cycles * 0.35:
        return "memory-port-limited"
    if queue_delay > compute_cycles * 0.25:
        return "queue-pressure-limited"
    return "compute-limited"

rows = []

for scenario in SCENARIOS:
    for groups in GROUPS:
        transfer_cycles = scenario["transfer_cycles_per_group"] * groups
        queue_delay = scenario["base_queue_delay"] * groups * groups
        total_cycles = scenario["compute_cycles"] + transfer_cycles + queue_delay

        throughput = groups / total_cycles
        communication_ratio = transfer_cycles / total_cycles
        queue_delay_ratio = queue_delay / total_cycles
        utilization_shift = {
            "compute_share": round(scenario["compute_cycles"] / total_cycles, 4),
            "communication_share": round(communication_ratio, 4),
            "queue_delay_share": round(queue_delay_ratio, 4),
        }

        rows.append({
            "scenario": scenario["name"],
            "simulated_compute_groups": groups,
            "compute_cycles": scenario["compute_cycles"],
            "synthetic_transfer_cycles": transfer_cycles,
            "queue_delay_cycles": queue_delay,
            "total_cycles": total_cycles,
            "simulated_throughput": round(throughput, 6),
            "communication_pressure_ratio": round(communication_ratio, 4),
            "queue_delay_ratio": round(queue_delay_ratio, 4),
            "utilization_shift": utilization_shift,
            "bottleneck_classification": classify(
                scenario["compute_cycles"],
                transfer_cycles,
                queue_delay,
            ),
        })

report = {
    "scope": "synthetic workload-level communication pressure model; not a real hardware/NCCL collective benchmark",
    "modeled_patterns": [
        "compute-heavy execution",
        "all-gather-style transfer pressure",
        "reduction-style transfer pressure",
        "memory/bus contention proxy",
        "queue growth under communication pressure",
    ],
    "results": rows,
}

Path("analysis/collective_bandwidth_sweep.json").write_text(json.dumps(report, indent=2))

md = ["# Communication Bottleneck Report\n"]
md.append("> Scope: synthetic workload-level communication pressure analysis, not a real hardware collective benchmark.\n")
md.append("| Scenario | Groups | Throughput | Total Cycles | Transfer Cycles | Queue Delay | Comm Ratio | Queue Ratio | Bottleneck |")
md.append("|---|---:|---:|---:|---:|---:|---:|---:|---|")

for r in rows:
    md.append(
        f"| {r['scenario']} | {r['simulated_compute_groups']} | "
        f"{r['simulated_throughput']:.6f} | {r['total_cycles']} | "
        f"{r['synthetic_transfer_cycles']} | {r['queue_delay_cycles']} | "
        f"{r['communication_pressure_ratio']} | {r['queue_delay_ratio']} | "
        f"{r['bottleneck_classification']} |"
    )

md.append("\n## Key observations\n")
md.append("- Compute-heavy workloads remain mostly compute-limited as simulated groups increase.")
md.append("- Transfer-heavy all-gather-style workloads shift toward communication-pressure limits.")
md.append("- Queue delay grows superlinearly in this synthetic model as more simulated compute groups contend for transfer capacity.")
md.append("- Reduction-style pressure shows an intermediate regime where communication and queue delay both influence throughput.")
md.append("- This report is intended for workload-level bottleneck reasoning, not real hardware timing prediction.\n")

md.append("## Safe interpretation\n")
md.append("AccelSim-Lite uses this experiment to compare compute-heavy and transfer-heavy execution patterns under synthetic communication pressure. It surfaces utilization shifts, queue-delay growth, and bottleneck classifications that are useful for reasoning about accelerator-style workload balance.\n")

Path("analysis/communication_bottleneck_report.md").write_text("\n".join(md) + "\n")

print(json.dumps(report, indent=2))
print("Generated analysis/collective_bandwidth_sweep.json")
print("Generated analysis/communication_bottleneck_report.md")
