#!/usr/bin/env python3
import json
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "build" / "trajectory-graph"
DATA_DIR = ROOT / "trajectory_graph_lite" / "data"
REPORT_DIR = ROOT / "trajectory_graph_lite" / "reports"

REPORT_DIR.mkdir(exist_ok=True)

cases = [
    ("connected_route", DATA_DIR / "connected_route.csv"),
    ("disconnected_route", DATA_DIR / "disconnected_route.csv"),
]

results = []

for name, path in cases:
    start = time.perf_counter()
    proc = subprocess.run(
        [str(BINARY), str(path)],
        text=True,
        capture_output=True
    )
    elapsed_ms = (time.perf_counter() - start) * 1000

    if proc.returncode != 0:
        raise SystemExit(proc.stderr)

    metrics = json.loads(proc.stdout)
    results.append({
        "case": name,
        "elapsed_ms": round(elapsed_ms, 4),
        **metrics
    })

summary = {
    "scope": "deterministic synthetic trajectory graph benchmark",
    "cases_run": len(results),
    "all_cases_passed": True,
    "results": results
}

(REPORT_DIR / "trajectory_graph_summary.json").write_text(
    json.dumps(summary, indent=2)
)

lines = [
    "# TrajectoryGraph-Lite Benchmark Report",
    "",
    "> Scope: deterministic synthetic route/trajectory analysis. "
    "This does not claim autonomous-driving production experience.",
    "",
    "| Case | Nodes | Edges | Components | Disconnected segments | "
    "Distance | Turns | Runtime ms |",
    "|---|---:|---:|---:|---:|---:|---:|---:|"
]

for result in results:
    lines.append(
        f"| {result['case']} | "
        f"{result['nodes']} | "
        f"{result['edges']} | "
        f"{result['connected_components']} | "
        f"{result['disconnected_segments']} | "
        f"{result['total_edge_distance']} | "
        f"{result['turn_transitions']} | "
        f"{result['elapsed_ms']} |"
    )

lines += [
    "",
    "## What this demonstrates",
    "",
    "- graph construction from deterministic trajectory points",
    "- connected-component analysis",
    "- disconnected road-segment detection",
    "- Euclidean edge-distance calculation",
    "- turn-transition analysis",
    "- repeatable C++ benchmark reporting",
    "",
    "## Claims boundary",
    "",
    "This is a synthetic graph/topology analysis module. "
    "It does not claim production mapping, autonomous-driving, "
    "road-network, or NVIDIA internal tooling experience."
]

(REPORT_DIR / "trajectory_graph_report.md").write_text(
    "\n".join(lines) + "\n"
)

print(json.dumps(summary, indent=2))
