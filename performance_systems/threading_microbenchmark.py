#!/usr/bin/env python3
import json
import queue
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT_JSON = ROOT / "threading_microbenchmark_results.json"
OUT_MD = ROOT / "threading_microbenchmark_report.md"

TOTAL_TASKS = 20000
WORKER_COUNTS = [1, 2, 4, 8]

def cpu_light_work(x):
    total = 0
    for i in range(40):
        total += (x * i) % 17
    return total

def run_worker_queue(worker_count):
    q = queue.Queue()
    for i in range(TOTAL_TASKS):
        q.put(i)

    completed = 0
    completed_lock = threading.Lock()

    def worker():
        nonlocal completed
        while True:
            try:
                item = q.get_nowait()
            except queue.Empty:
                return

            cpu_light_work(item)

            with completed_lock:
                completed += 1

            q.task_done()

    start = time.perf_counter()
    threads = [threading.Thread(target=worker) for _ in range(worker_count)]

    for t in threads:
        t.start()
    for t in threads:
        t.join()

    elapsed_ms = (time.perf_counter() - start) * 1000
    throughput = completed / (elapsed_ms / 1000)

    return {
        "worker_count": worker_count,
        "tasks": TOTAL_TASKS,
        "elapsed_ms": round(elapsed_ms, 4),
        "throughput_tasks_per_sec": round(throughput, 2),
        "completed": completed
    }

results = [run_worker_queue(w) for w in WORKER_COUNTS]
baseline = results[0]["throughput_tasks_per_sec"]

for r in results:
    r["throughput_relative_to_single_worker"] = round(
        r["throughput_tasks_per_sec"] / baseline, 4
    )

payload = {
    "scope": "Python threading/concurrency microbenchmark for queue-worker behavior; not a production scheduler benchmark",
    "results": results
}
OUT_JSON.write_text(json.dumps(payload, indent=2))

lines = [
    "# Threading / Queue Concurrency Microbenchmark",
    "",
    "> Scope: queue-worker concurrency microbenchmark. This is not a production scheduler or lock-free systems benchmark.",
    "",
    "| Workers | Tasks | Elapsed ms | Throughput tasks/sec | Relative throughput |",
    "|---:|---:|---:|---:|---:|"
]

for r in results:
    lines.append(
        f"| {r['worker_count']} | {r['tasks']} | {r['elapsed_ms']} | "
        f"{r['throughput_tasks_per_sec']} | {r['throughput_relative_to_single_worker']}x |"
    )

lines += [
    "",
    "## What this means",
    "",
    "This experiment models a shared task-queue worker pattern and reports how throughput changes as worker count increases.",
    "",
    "It is useful for discussing:",
    "- queue contention",
    "- worker scaling",
    "- shared work distribution",
    "- concurrency overhead",
    "- throughput measurement methodology",
    "",
    "## Claims boundary",
    "",
    "This is an educational concurrency microbenchmark. It does not claim production thread-pool engineering, lock-free queue implementation, or HFT-grade latency measurement."
]

OUT_MD.write_text("\n".join(lines) + "\n")

print(f"Generated {OUT_JSON}")
print(f"Generated {OUT_MD}")
