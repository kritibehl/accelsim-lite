# Queue Batching Optimization Report

## Project scope

AccelSim-Lite is a deterministic C++ performance-regression lab. It is not
the established Accel-Sim GPU simulator and does not claim hardware-accurate
GPU modeling.

## Measured bottleneck

The original executor removed one task from the shared queue per mutex
acquisition. With eight workers, the shared queue generated thousands of lock
acquisitions per run and limited multithreaded scaling.

## Optimization patch

The optimized executor removes up to eight tasks during one mutex acquisition
and processes the batch outside the critical section.

Correctness is preserved through:

- identical task counts
- identical deterministic checksums
- GoogleTest coverage
- ASan validation
- UBSan validation
- ThreadSanitizer validation

## Methodology

- warmups: 5
- measured iterations: 25
- workers: 8
- workload: 1,000,000 items
- access pattern: local
- baseline queue batch: 1
- candidate queue batch: 8
- regression threshold: 10.0%

## Results

| Metric | Baseline | Candidate | Change |
|---|---:|---:|---:|
| Median runtime | 4.3072 ms | 2.9207 ms | 32.19% reduction |
| Cross-run p95 runtime | 4.7445 ms | 5.5792 ms | 17.59% |
| Cross-run p99 runtime | 6.2025 ms | 7.3047 ms | 17.77% |
| Median task p95 | 376.2625 us | 106.8498 us | 71.60% reduction |
| Median task p99 | 427.8205 us | 144.2870 us | 66.27% reduction |
| Median throughput | 232168905.66 items/s | 342377813.92 items/s | 47.47% increase |
| Median lock acquisitions | 3915 | 605 | 84.55% reduction |
| Runtime variance | 0.389889 | 1.429208 | 266.57% |

## Regression-gate decision

**PASS**

Primary release metrics:

- median runtime
- median task p95 latency
- deterministic checksum

## Stability warnings

- cross-run p95 runtime increased beyond threshold
- cross-run p99 runtime increased beyond threshold
- run-to-run variance increased beyond threshold

The candidate substantially improves normal execution and task-tail latency,
but its increased cross-run variance remains documented rather than hidden.

## Bottleneck conclusion

Queue batching reduced synchronization frequency by 84.55%,
which corresponded with a 32.19% reduction in median runtime
and a 71.60% reduction in median task p95 latency.

## Claims boundary

These are local Apple Silicon measurements from deterministic synthetic
workloads. They are not hardware-accurate GPU simulation or production
infrastructure capacity claims.
