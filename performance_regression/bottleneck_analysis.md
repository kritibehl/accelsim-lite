# Bottleneck Analysis

## Biggest bottlenecks

| Bottleneck | Evidence | Recommendation |
|---|---|---|
| cache locality | memory-pressure latency increased from 23.8 to 86.64 cycles | reduce pointer-heavy access and improve contiguous layout |
| metadata lookup | p95 increased from 410ns to 620ns | reduce lookup-path probes and preserve hot metadata |
| scheduler contention | p95 increased from 1800ns to 2200ns | reduce shared queue contention or partition work |

## Claims boundary

This is an educational performance-analysis artifact and does not claim production hardware profiling.
