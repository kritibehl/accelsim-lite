#!/usr/bin/env python3
import json
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "build" / "storage-datapath"
DATA = ROOT / "storage_datapath" / "data"
REPORTS = ROOT / "storage_datapath" / "reports"

REPORTS.mkdir(exist_ok=True)

cases = [
    ("baseline", DATA / "baseline_workload.csv"),
    ("cache_pressure", DATA / "cache_pressure_workload.csv"),
    ("queue_pressure", DATA / "queue_pressure_workload.csv"),
]

results = []

for name, workload in cases:
    start = time.perf_counter()

    proc = subprocess.run(
        [str(BINARY), str(workload)],
        text=True,
        capture_output=True
    )

    wall_time_ms = (time.perf_counter() - start) * 1000.0

    if proc.returncode != 0:
        raise SystemExit(proc.stderr)

    metrics = json.loads(proc.stdout)
    results.append({
        "case": name,
        "wall_time_ms": round(wall_time_ms, 4),
        **metrics
    })

summary = {
    "scope": "deterministic simulated storage datapath benchmark",
    "cases_run": len(results),
    "all_cases_passed": True,
    "results": results
}

(REPORTS / "storage_datapath_summary.json").write_text(
    json.dumps(summary, indent=2)
)

lines = [
    "# Storage Datapath Simulation Report",
    "",
    "> Scope: simulated storage-style request processing. "
    "This does not claim filesystem, kernel, device, or Azure Storage implementation experience.",
    "",
    "| Case | Requests | Queue depth | Cache hits | Cache misses | "
    "Throughput req/s | p50 us | p95 us | p99 us | Bottleneck |",
    "|---|---:|---:|---:|---:|---:|---:|---:|---:|---|"
]

for r in results:
    lines.append(
        f"| {r['case']} | "
        f"{r['total_requests']} | "
        f"{r['max_queue_depth']} | "
        f"{r['cache_hits']} | "
        f"{r['cache_misses']} | "
        f"{r['throughput_requests_per_sec']} | "
        f"{r['p50_latency_us']} | "
        f"{r['p95_latency_us']} | "
        f"{r['p99_latency_us']} | "
        f"{r['bottleneck']} |"
    )

lines += [
    "",
    "## What this demonstrates",
    "",
    "- deterministic read/write request queues",
    "- queue-depth tracking",
    "- cache hit/miss behavior",
    "- simulated memory-bandwidth pressure",
    "- throughput measurement",
    "- p50/p95/p99 latency reporting",
    "- bottleneck classification",
    "",
    "## Claims boundary",
    "",
    "This is a storage-style datapath simulation module. "
    "It does not claim kernel, filesystem, device virtualization, "
    "hardware offload, or Azure internal storage infrastructure experience."
]

(REPORTS / "storage_datapath_report.md").write_text(
    "\n".join(lines) + "\n"
)

print(json.dumps(summary, indent=2))
