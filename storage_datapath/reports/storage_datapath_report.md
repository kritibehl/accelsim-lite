# Storage Datapath Simulation Report

> Scope: simulated storage-style request processing. This does not claim filesystem, kernel, device, or Azure Storage implementation experience.

| Case | Requests | Queue depth | Cache hits | Cache misses | Throughput req/s | p50 us | p95 us | p99 us | Bottleneck |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| baseline | 6 | 6 | 4 | 2 | 97560.976 | 9.0 | 19.0 | 19.8 | balanced |
| cache_pressure | 8 | 8 | 1 | 7 | 45977.011 | 21.25 | 27.6 | 28.72 | cache_miss_pressure |
| queue_pressure | 12 | 8 | 3 | 9 | 42253.521 | 24.75 | 31.4 | 32.28 | queue_depth_pressure |

## What this demonstrates

- deterministic read/write request queues
- queue-depth tracking
- cache hit/miss behavior
- simulated memory-bandwidth pressure
- throughput measurement
- p50/p95/p99 latency reporting
- bottleneck classification

## Claims boundary

This is a storage-style datapath simulation module. It does not claim kernel, filesystem, device virtualization, hardware offload, or Azure internal storage infrastructure experience.
