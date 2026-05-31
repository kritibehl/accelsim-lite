# Performance Regression Gate Report

> Scope: baseline-vs-candidate benchmark comparison for C++ runtime/performance experiments. This is not production performance certification.

| Benchmark | Baseline p95 ns | Candidate p95 ns | p95 change | Throughput change | Decision |
|---|---:|---:|---:|---:|---|
| metadata_lookup | 410 | 620 | 51.22% | -12.5% | FAIL |
| cache_locality_scan | 1200 | 1180 | -1.67% | 1.18% | PASS |
| threading_queue_worker | 1800 | 2200 | 22.22% | -7.87% | PASS |

## Failure diagnostics

- `metadata_lookup`: p95 latency regression 51.22% > 25.0%; throughput drop -12.50% < -10.0%

## Final decision

`FAIL`

## Claims boundary

This gate compares checked-in benchmark summaries and demonstrates regression-gating methodology. It does not claim production-scale benchmarking.
