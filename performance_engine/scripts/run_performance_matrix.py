#!/usr/bin/env python3

import csv
import json
import platform
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "build" / "performance-benchmark"
REPORT_DIR = ROOT / "performance_engine" / "reports"

REPORT_DIR.mkdir(parents=True, exist_ok=True)

workload_sizes = [1_000, 10_000, 100_000]
worker_counts = [1, 2, 4, 8]
patterns = ["local", "unfriendly"]

rows = []

for workload_size in workload_sizes:
    for pattern in patterns:
        commands = [
            (
                "single",
                1,
                [
                    str(BINARY),
                    "single",
                    "1",
                    str(workload_size),
                    pattern,
                ],
            )
        ]

        commands.extend(
            (
                "multi",
                worker_count,
                [
                    str(BINARY),
                    "multi",
                    str(worker_count),
                    str(workload_size),
                    pattern,
                ],
            )
            for worker_count in worker_counts
        )

        for requested_mode, worker_count, command in commands:
            completed = subprocess.run(
                command,
                check=True,
                text=True,
                capture_output=True,
            )

            parsed = list(
                csv.DictReader(completed.stdout.splitlines())
            )

            if len(parsed) != 1:
                raise RuntimeError(
                    f"Unexpected benchmark output: {completed.stdout}"
                )

            row = parsed[0]
            row["requested_mode"] = requested_mode
            row["requested_workers"] = worker_count
            rows.append(row)

summary = {
    "scope": "deterministic C++ runtime performance benchmark",
    "platform": platform.platform(),
    "processor": platform.processor(),
    "cases_run": len(rows),
    "all_cases_passed": True,
    "results": rows,
}

json_path = REPORT_DIR / "performance_matrix.json"
json_path.write_text(json.dumps(summary, indent=2))

csv_path = REPORT_DIR / "performance_matrix.csv"

with csv_path.open("w", newline="") as handle:
    writer = csv.DictWriter(
        handle,
        fieldnames=rows[0].keys(),
    )
    writer.writeheader()
    writer.writerows(rows)

lines = [
    "# Multithreaded Performance Benchmark",
    "",
    "> Scope: deterministic synthetic C++ workloads. "
    "Results are machine-specific and are not production capacity claims.",
    "",
    "## Benchmark environment",
    "",
    f"- Platform: `{summary['platform']}`",
    f"- Processor: `{summary['processor'] or 'not reported by Python'}`",
    f"- Cases: `{summary['cases_run']}`",
    "",
    "## Results",
    "",
    "| Mode | Workers | Workload | Pattern | Throughput items/s | "
    "p50 µs | p95 µs | p99 µs | Contention rate | Allocations | Bottleneck |",
    "|---|---:|---:|---|---:|---:|---:|---:|---:|---:|---|",
]

for row in rows:
    lines.append(
        f"| {row['mode']} | "
        f"{row['worker_count']} | "
        f"{row['workload_size']} | "
        f"{row['access_pattern']} | "
        f"{float(row['throughput_items_per_second']):.2f} | "
        f"{float(row['p50_task_latency_us']):.2f} | "
        f"{float(row['p95_task_latency_us']):.2f} | "
        f"{float(row['p99_task_latency_us']):.2f} | "
        f"{float(row['contention_rate']):.4f} | "
        f"{row['simulated_allocations']} | "
        f"{row['bottleneck']} |"
    )

lines.extend(
    [
        "",
        "## Metrics",
        "",
        "- throughput measures processed data items per second",
        "- task latency includes queue wait plus task execution",
        "- lock contention counts failed immediate mutex acquisitions",
        "- allocation counts represent benchmark-owned temporary buffers",
        "- cache-local and cache-unfriendly cases process equivalent data",
        "",
        "## Claims boundary",
        "",
        "This benchmark uses deterministic synthetic workloads. "
        "It does not represent production trading, exchange, market-data, "
        "or Citadel internal infrastructure.",
    ]
)

(
    REPORT_DIR / "multithreaded_performance_report.md"
).write_text("\n".join(lines) + "\n")

print(json.dumps(summary, indent=2))
