#!/usr/bin/env python3
import json
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT_JSON = ROOT / "cache_locality_results.json"
OUT_MD = ROOT / "cache_locality_report.md"

N = 250_000

def time_it(fn):
    start = time.perf_counter()
    result = fn()
    end = time.perf_counter()
    return result, (end - start) * 1000

def contiguous_sum():
    data = list(range(N))
    return sum(data)

def strided_sum():
    data = list(range(N))
    total = 0
    for i in range(0, N, 8):
        total += data[i]
    return total

def pointer_heavy_sum():
    nodes = [{"value": i, "next": i + 1} for i in range(N)]
    total = 0
    idx = 0
    steps = 0
    while idx < N and steps < N:
        total += nodes[idx]["value"]
        idx = nodes[idx]["next"]
        steps += 1
    return total

def aos_layout_sum():
    rows = [{"x": i, "y": i * 2, "z": i * 3} for i in range(N)]
    return sum(r["x"] for r in rows)

def soa_layout_sum():
    xs = list(range(N))
    ys = [i * 2 for i in range(N)]
    zs = [i * 3 for i in range(N)]
    return sum(xs)

experiments = [
    ("contiguous_scan", contiguous_sum, "contiguous list traversal"),
    ("strided_scan", strided_sum, "strided access pattern"),
    ("pointer_heavy_scan", pointer_heavy_sum, "pointer-like dictionary chain traversal"),
    ("array_of_structs", aos_layout_sum, "array-of-structs-style object layout"),
    ("struct_of_arrays", soa_layout_sum, "structure-of-arrays-style layout")
]

results = []
for name, fn, desc in experiments:
    value, elapsed_ms = time_it(fn)
    results.append({
        "experiment": name,
        "description": desc,
        "items": N,
        "elapsed_ms": round(elapsed_ms, 4),
        "result_checksum": value
    })

baseline = next(r for r in results if r["experiment"] == "contiguous_scan")["elapsed_ms"]
for r in results:
    r["relative_to_contiguous_x"] = round(r["elapsed_ms"] / baseline, 4) if baseline else None

payload = {
    "scope": "Python cache/locality-adjacent experiment; not hardware cache measurement",
    "results": results
}
OUT_JSON.write_text(json.dumps(payload, indent=2))

lines = [
    "# Cache / Locality Experiment Report",
    "",
    "> Scope: locality-adjacent experiment for access-pattern reasoning. This is not hardware cache measurement or CPU microarchitecture benchmarking.",
    "",
    "| Experiment | Description | Items | Elapsed ms | Relative to contiguous |",
    "|---|---|---:|---:|---:|"
]
for r in results:
    lines.append(
        f"| {r['experiment']} | {r['description']} | {r['items']} | "
        f"{r['elapsed_ms']} | {r['relative_to_contiguous_x']}x |"
    )

lines += [
    "",
    "## Interpretation",
    "",
    "- Contiguous scans model locality-friendly access.",
    "- Strided and pointer-heavy scans model reduced locality and indirection.",
    "- AoS vs SoA captures memory-layout tradeoff reasoning used in systems/performance work.",
    "",
    "## Claims boundary",
    "",
    "This experiment is Python-level and intended for systems reasoning. It does not claim hardware cache-counter measurement."
]
OUT_MD.write_text("\n".join(lines) + "\n")

print(f"Generated {OUT_JSON}")
print(f"Generated {OUT_MD}")
