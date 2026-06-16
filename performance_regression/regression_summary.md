# Performance Regression Summary

| Metric | Baseline | Candidate | Result |
|---|---:|---:|---|
| instruction lifecycle pass rate | 1.0 | 1.0 | pass |
| scheduler p95 latency | 1800 ns | 2200 ns | review |
| memory-pressure latency | 23.8 cycles | 86.64 cycles | regression |
| metadata lookup p95 | 410 ns | 620 ns | regression |
| throughput | 2400000 ops/sec | 2100000 ops/sec | regression |

## Decision

`needs_review`

## Scope

Baseline-vs-candidate runtime validation summary for simulator-derived performance metrics.
