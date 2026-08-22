#!/usr/bin/env python3

import csv
import json
import statistics
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]

PTHREAD_BIN = (
    ROOT / "build" / "pthread-queue-benchmark"
)

CPP_BIN = (
    ROOT / "build" / "performance-benchmark"
)

REPORT_DIR = (
    ROOT
    / "benchmarks"
    / "pthreads_queue"
    / "reports"
)

REPORT_DIR.mkdir(
    parents=True,
    exist_ok=True,
)

WORKERS = [1, 2, 4, 8]
WORKLOADS = [10_000, 100_000, 1_000_000]

WARMUPS = 3
ITERATIONS = 15


def run_csv(command):
    completed = subprocess.run(
        command,
        check=True,
        capture_output=True,
        text=True,
    )

    rows = list(
        csv.DictReader(
            completed.stdout.splitlines()
        )
    )

    if len(rows) != 1:
        raise RuntimeError(
            completed.stdout
        )

    return rows[0]


results = []

for workload in WORKLOADS:
    for workers in WORKERS:
        pthread_samples = []
        cpp_samples = []

        for _ in range(WARMUPS):
            run_csv(
                [
                    str(PTHREAD_BIN),
                    str(workers),
                    str(workload),
                ]
            )

            run_csv(
                [
                    str(CPP_BIN),
                    "multi",
                    str(workers),
                    str(workload),
                    "local",
                    "1",
                ]
            )

        for _ in range(ITERATIONS):
            c_row = run_csv(
                [
                    str(PTHREAD_BIN),
                    str(workers),
                    str(workload),
                ]
            )

            cpp_row = run_csv(
                [
                    str(CPP_BIN),
                    "multi",
                    str(workers),
                    str(workload),
                    "local",
                    "1",
                ]
            )

            if c_row["tasks_processed"] != cpp_row["tasks_processed"]:
                raise RuntimeError(
                    f"task-count mismatch: workers={workers}, "
                    f"workload={workload}"
                )

            if c_row["checksum"] != cpp_row["checksum"]:
                raise RuntimeError(
                    f"checksum mismatch: workers={workers}, "
                    f"workload={workload}"
                )

            pthread_samples.append(c_row)
            cpp_samples.append(cpp_row)

        def median_metric(
            samples,
            key,
        ):
            return statistics.median(
                float(sample[key])
                for sample in samples
            )

        pthread_summary = {
            "implementation": "C pthreads",
            "workers": workers,
            "workload": workload,
            "median_throughput":
                median_metric(
                    pthread_samples,
                    "throughput_items_per_second",
                ),
            "median_p95_us":
                median_metric(
                    pthread_samples,
                    "p95_task_latency_us",
                ),
            "median_p99_us":
                median_metric(
                    pthread_samples,
                    "p99_task_latency_us",
                ),
            "median_contention":
                median_metric(
                    pthread_samples,
                    "contention_rate",
                ),
            "median_lock_acquisitions":
                median_metric(
                    pthread_samples,
                    "lock_acquisitions",
                ),
            "median_allocations":
                median_metric(
                    pthread_samples,
                    "allocations",
                ),
            "checksum":
                pthread_samples[0]["checksum"],
        }

        cpp_summary = {
            "implementation": "C++ std::thread",
            "workers": workers,
            "workload": workload,
            "median_throughput":
                median_metric(
                    cpp_samples,
                    "throughput_items_per_second",
                ),
            "median_p95_us":
                median_metric(
                    cpp_samples,
                    "p95_task_latency_us",
                ),
            "median_p99_us":
                median_metric(
                    cpp_samples,
                    "p99_task_latency_us",
                ),
            "median_contention":
                median_metric(
                    cpp_samples,
                    "contention_rate",
                ),
            "median_lock_acquisitions":
                median_metric(
                    cpp_samples,
                    "lock_acquisitions",
                ),
            "median_allocations":
                median_metric(
                    cpp_samples,
                    "simulated_allocations",
                ),
            "checksum":
                cpp_samples[0]["checksum"],
        }

        results.extend(
            [
                pthread_summary,
                cpp_summary,
            ]
        )

output = {
    "scope":
        "deterministic C pthread vs C++ threading comparison",
    "warmups": WARMUPS,
    "iterations": ITERATIONS,
    "results": results,
}

(
    REPORT_DIR
    / "pthreads_vs_cpp.json"
).write_text(
    json.dumps(
        output,
        indent=2,
    )
)

lines = [
    "# C pthreads vs C++ std::thread Benchmark",
    "",
    "> Scope: deterministic synthetic concurrency workloads.",
    "",
    f"- warmups: {WARMUPS}",
    f"- measured iterations: {ITERATIONS}",
    "",
    "| Implementation | Workers | Workload | "
    "Median throughput | Median p95 us | "
    "Median p99 us | Contention | "
    "Lock acquisitions | Allocations |",
    "|---|---:|---:|---:|---:|---:|---:|---:|---:|",
]

for result in results:
    lines.append(
        f"| {result['implementation']} | "
        f"{result['workers']} | "
        f"{result['workload']} | "
        f"{result['median_throughput']:.2f} | "
        f"{result['median_p95_us']:.2f} | "
        f"{result['median_p99_us']:.2f} | "
        f"{result['median_contention']:.4f} | "
        f"{result['median_lock_acquisitions']:.0f} | "
        f"{result['median_allocations']:.0f} |"
    )

lines += [
    "",
    "## What this demonstrates",
    "",
    "- POSIX thread creation and joining",
    "- pthread mutex synchronization",
    "- condition variables",
    "- bounded work queues",
    "- malloc/free ownership",
    "- C pointers and structs",
    "- contention instrumentation",
    "- deterministic cross-language benchmarking",
    "",
    "## Claims boundary",
    "",
    "This module demonstrates focused POSIX-threaded C systems work. "
    "It does not by itself establish broad C fluency, kernel-development "
    "experience, or Darwin runtime implementation experience.",
]

(
    REPORT_DIR
    / "pthreads_vs_cpp.md"
).write_text(
    "\n".join(lines) + "\n"
)

print(
    json.dumps(
        output,
        indent=2,
    )
)
