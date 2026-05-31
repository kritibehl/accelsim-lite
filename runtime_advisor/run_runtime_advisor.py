#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_JSON = Path("runtime_advisor/runtime_advisor_report.json")
OUT_MD = Path("runtime_advisor/runtime_advisor_report.md")

signals = [
    {
        "workload": "pointer_chasing",
        "bottleneck": "cache locality",
        "evidence": "pointer_chasing shows 21.66x slowdown vs all-L1 access in simulated cache hierarchy study",
        "recommendation": "reduce pointer-heavy traversal, improve data locality, or restructure hot data into contiguous layouts",
        "expected_impact": "lower average memory-access latency"
    },
    {
        "workload": "metadata_lookup",
        "bottleneck": "runtime metadata lookup",
        "evidence": "metadata_lookup regressed from 410ns to 620ns p95 in candidate benchmark comparison",
        "recommendation": "review lookup-path changes, reduce unnecessary map/tree probes, and preserve hot metadata in faster lookup structures",
        "expected_impact": "reduce p95 latency and avoid throughput regression"
    },
    {
        "workload": "threading_queue_worker",
        "bottleneck": "queue contention",
        "evidence": "worker scaling flattened after 2 workers in queue-worker microbenchmark",
        "recommendation": "reduce shared queue contention, batch task pulls, or partition work per worker",
        "expected_impact": "improve scaling under higher worker counts"
    },
    {
        "workload": "decode_heavy_inference",
        "bottleneck": "dependency pressure",
        "evidence": "decode-heavy simulated inference profile remained dependency-bound after memory-port scaling",
        "recommendation": "focus on dependency scheduling and latency-sensitive decode path behavior rather than only adding memory bandwidth",
        "expected_impact": "better latency reasoning for decode-heavy serving traces"
    }
]

payload = {
    "scope": "educational runtime advisor over AccelSim-Lite benchmark outputs",
    "advisor_outputs": signals
}
OUT_JSON.write_text(json.dumps(payload, indent=2))

lines = [
    "# Runtime Advisor",
    "",
    "> Scope: readable diagnosis layer over AccelSim-Lite benchmark outputs. Recommendations are educational and simulator-derived.",
    "",
    "| Workload | Bottleneck | Recommendation | Expected impact |",
    "|---|---|---|---|"
]

for s in signals:
    lines.append(
        f"| {s['workload']} | {s['bottleneck']} | {s['recommendation']} | {s['expected_impact']} |"
    )

lines += [
    "",
    "## Example advisor output",
    "",
    "```json",
    json.dumps(signals[0], indent=2),
    "```",
    "",
    "## Claims boundary",
    "",
    "This advisor summarizes simulator-derived and benchmark-study outputs. It does not claim production autotuning, compiler optimization, or hardware profiling."
]

OUT_MD.write_text("\n".join(lines) + "\n")

print(f"Generated {OUT_JSON}")
print(f"Generated {OUT_MD}")
