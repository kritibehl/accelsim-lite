import json
from pathlib import Path

src = Path("reports/analysis/workload_comparison.json")
data = json.loads(src.read_text())

def classify(row):
    bottleneck = row["top_bottleneck"]
    stalls = row.get("stall_counts", {})

    if bottleneck == "NoMemoryPort":
        return "memory-bound"
    if bottleneck == "NoComputeUnit":
        return "compute-bound"
    if bottleneck == "WaitingDependency":
        if stalls.get("NoComputeUnit", 0) > 0:
            return "dependency-bound with compute contention"
        if stalls.get("NoMemoryPort", 0) > 0:
            return "dependency-bound with memory pressure"
        return "dependency-bound"
    if bottleneck in {"DispatchQueueFull", "ReadyQueueFull"}:
        return "queue-bound"
    return "mixed"

classified = []
for row in data["workloads"]:
    item = dict(row)
    item["classification"] = classify(row)
    classified.append(item)

out = {
    "classified_workloads": classified,
    "summary": {
        r["workload"]: r["classification"] for r in classified
    }
}

Path("reports/analysis/workload_classification.json").write_text(json.dumps(out, indent=2))

md = ["# Workload Classification\n", "| Workload | Classification | Top Bottleneck |", "|---|---|---|"]
for r in classified:
    md.append(f"| {r['workload']} | {r['classification']} | {r['top_bottleneck']} |")

Path("reports/analysis/workload_classification.md").write_text("\n".join(md) + "\n")

print(json.dumps(out, indent=2))
