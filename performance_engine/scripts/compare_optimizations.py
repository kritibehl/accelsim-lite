#!/usr/bin/env python3

import csv
import json
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REPORT_DIR = ROOT / "performance_engine" / "reports"
REPORT_DIR.mkdir(parents=True, exist_ok=True)

profiles = {
    "O0": "-O0",
    "O2": "-O2",
    "O3": "-O3",
}

results = []

for name, flag in profiles.items():
    build_dir = ROOT / f"build-{name.lower()}"

    if build_dir.exists():
        shutil.rmtree(build_dir)

    subprocess.run(
        [
            "cmake",
            "-S",
            str(ROOT),
            "-B",
            str(build_dir),
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_CXX_FLAGS_RELEASE={flag} -DNDEBUG",
        ],
        check=True,
    )

    subprocess.run(
        [
            "cmake",
            "--build",
            str(build_dir),
            "--target",
            "performance-benchmark",
            "-j",
        ],
        check=True,
    )

    binary = build_dir / "performance-benchmark"

    completed = subprocess.run(
        [
            str(binary),
            "multi",
            "4",
            "100000",
            "local",
        ],
        check=True,
        text=True,
        capture_output=True,
    )

    parsed = list(
        csv.DictReader(completed.stdout.splitlines())
    )

    if len(parsed) != 1:
        raise RuntimeError(
            f"Unexpected benchmark output for {name}"
        )

    row = parsed[0]
    row["optimization"] = name
    row["compiler_flag"] = flag
    results.append(row)

summary = {
    "scope": "compiler optimization comparison",
    "workload_size": 100000,
    "worker_count": 4,
    "access_pattern": "cache_local",
    "results": results,
}

(
    REPORT_DIR / "compiler_optimization_results.json"
).write_text(json.dumps(summary, indent=2))

lines = [
    "# Compiler Optimization Comparison",
    "",
    "> Results are machine-specific deterministic benchmark observations.",
    "",
    "| Optimization | Flag | Runtime ms | Throughput items/s | "
    "p50 µs | p95 µs | p99 µs |",
    "|---|---|---:|---:|---:|---:|---:|",
]

for row in results:
    lines.append(
        f"| {row['optimization']} | "
        f"`{row['compiler_flag']}` | "
        f"{float(row['elapsed_ms']):.4f} | "
        f"{float(row['throughput_items_per_second']):.2f} | "
        f"{float(row['p50_task_latency_us']):.2f} | "
        f"{float(row['p95_task_latency_us']):.2f} | "
        f"{float(row['p99_task_latency_us']):.2f} |"
    )

lines.extend(
    [
        "",
        "## Interpretation",
        "",
        "- `-O0` provides an unoptimized development baseline",
        "- `-O2` enables broadly useful compiler optimizations",
        "- `-O3` enables more aggressive optimization",
        "- results should be compared on the same machine",
        "- repeated runs are needed before making strong performance claims",
    ]
)

(
    REPORT_DIR / "compiler_optimization_report.md"
).write_text("\n".join(lines) + "\n")

print(json.dumps(summary, indent=2))
