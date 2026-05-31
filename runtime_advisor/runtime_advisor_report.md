# Runtime Advisor

> Scope: readable diagnosis layer over AccelSim-Lite benchmark outputs. Recommendations are educational and simulator-derived.

| Workload | Bottleneck | Recommendation | Expected impact |
|---|---|---|---|
| pointer_chasing | cache locality | reduce pointer-heavy traversal, improve data locality, or restructure hot data into contiguous layouts | lower average memory-access latency |
| metadata_lookup | runtime metadata lookup | review lookup-path changes, reduce unnecessary map/tree probes, and preserve hot metadata in faster lookup structures | reduce p95 latency and avoid throughput regression |
| threading_queue_worker | queue contention | reduce shared queue contention, batch task pulls, or partition work per worker | improve scaling under higher worker counts |
| decode_heavy_inference | dependency pressure | focus on dependency scheduling and latency-sensitive decode path behavior rather than only adding memory bandwidth | better latency reasoning for decode-heavy serving traces |

## Example advisor output

```json
{
  "workload": "pointer_chasing",
  "bottleneck": "cache locality",
  "evidence": "pointer_chasing shows 21.66x slowdown vs all-L1 access in simulated cache hierarchy study",
  "recommendation": "reduce pointer-heavy traversal, improve data locality, or restructure hot data into contiguous layouts",
  "expected_impact": "lower average memory-access latency"
}
```

## Claims boundary

This advisor summarizes simulator-derived and benchmark-study outputs. It does not claim production autotuning, compiler optimization, or hardware profiling.
