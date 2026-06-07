# Regression Explorer

> Scope: release-to-release comparison for AccelSim-Lite benchmark and build-health signals.

## Executive summary

```json
{
  "release_a": "v1.2",
  "release_b": "v1.3",
  "regressions": 3,
  "failed_build_delta": 5,
  "flaky_test_delta": 1,
  "root_cause": "metadata lookup path regression",
  "decision": "block_release"
}
```

## Benchmark comparison

| Metric | Release A | Release B | Change | Classification |
|---|---:|---:|---:|---|
| metadata_lookup_p95_ns | 410 | 620 | 51.22% | regression |
| metadata_lookup_throughput_ops_sec | 2400000 | 2100000 | -12.5% | regression |
| cache_locality_latency_cycles | 23.8 | 86.64 | 264.03% | regression |
| threading_queue_throughput_ops_sec | 445000 | 410000 | -7.87% | neutral |

## Build-health comparison

- Failed builds: 2 → 7 (+5)
- Flaky tests: 1 → 2 (+1)

## Decision

`block_release`

## What this means

The Regression Explorer turns performance/build data into a release decision by comparing benchmark drift, build-health movement, root-cause signals, and release risk.

## Claims boundary

This is an educational release-regression workflow over simulator-derived benchmark/build-health data. It does not claim production release ownership.
