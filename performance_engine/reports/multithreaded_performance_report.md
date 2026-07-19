# Multithreaded Performance Benchmark

> Scope: deterministic synthetic C++ workloads. Results are machine-specific and are not production capacity claims.

## Benchmark environment

- Platform: `macOS-26.3.1-arm64-arm-64bit`
- Processor: `arm`
- Cases: `30`

## Results

| Mode | Workers | Workload | Pattern | Throughput items/s | p50 µs | p95 µs | p99 µs | Contention rate | Allocations | Bottleneck |
|---|---:|---:|---|---:|---:|---:|---:|---:|---:|---|
| single_threaded | 1 | 1000 | cache_local | 35242290.75 | 6.90 | 7.66 | 7.77 | 0.0000 | 4 | allocation_pressure |
| multithreaded | 1 | 1000 | cache_local | 16010502.89 | 26.15 | 35.65 | 36.46 | 0.0000 | 4 | queue_wait_pressure |
| multithreaded | 2 | 1000 | cache_local | 18278528.21 | 15.67 | 20.31 | 20.66 | 0.1667 | 4 | lock_contention |
| multithreaded | 4 | 1000 | cache_local | 8998875.14 | 43.92 | 46.85 | 47.10 | 0.2500 | 4 | lock_contention |
| multithreaded | 8 | 1000 | cache_local | 6119314.39 | 21.46 | 47.67 | 50.80 | 0.2500 | 4 | lock_contention |
| single_threaded | 1 | 1000 | cache_unfriendly | 25052610.48 | 9.56 | 11.38 | 11.61 | 0.0000 | 4 | cache_locality_pressure |
| multithreaded | 1 | 1000 | cache_unfriendly | 13753644.72 | 35.40 | 48.35 | 49.54 | 0.0000 | 4 | cache_locality_pressure |
| multithreaded | 2 | 1000 | cache_unfriendly | 15635016.18 | 28.67 | 34.44 | 34.65 | 0.0000 | 4 | cache_locality_pressure |
| multithreaded | 4 | 1000 | cache_unfriendly | 13896995.47 | 17.35 | 21.16 | 21.50 | 0.5000 | 4 | cache_locality_pressure |
| multithreaded | 8 | 1000 | cache_unfriendly | 7495296.70 | 24.58 | 25.16 | 25.23 | 0.1667 | 4 | cache_locality_pressure |
| single_threaded | 1 | 10000 | cache_local | 34502627.38 | 7.25 | 7.53 | 8.40 | 0.0000 | 40 | allocation_pressure |
| multithreaded | 1 | 10000 | cache_local | 29382986.66 | 174.50 | 305.28 | 312.95 | 0.0000 | 40 | queue_wait_pressure |
| multithreaded | 2 | 10000 | cache_local | 47440130.56 | 99.19 | 168.44 | 173.62 | 0.0000 | 40 | queue_wait_pressure |
| multithreaded | 4 | 10000 | cache_local | 64136688.11 | 56.44 | 86.89 | 89.09 | 0.0909 | 40 | queue_wait_pressure |
| multithreaded | 8 | 10000 | cache_local | 40288627.73 | 37.12 | 72.66 | 89.11 | 0.1250 | 40 | lock_contention |
| single_threaded | 1 | 10000 | cache_unfriendly | 25809182.39 | 9.54 | 11.23 | 13.72 | 0.0000 | 40 | cache_locality_pressure |
| multithreaded | 1 | 10000 | cache_unfriendly | 23052576.01 | 223.54 | 394.49 | 404.57 | 0.0000 | 40 | cache_locality_pressure |
| multithreaded | 2 | 10000 | cache_unfriendly | 42119982.98 | 118.19 | 198.15 | 203.16 | 0.0238 | 40 | cache_locality_pressure |
| multithreaded | 4 | 10000 | cache_unfriendly | 60990857.47 | 65.06 | 99.93 | 102.47 | 0.0455 | 40 | cache_locality_pressure |
| multithreaded | 8 | 10000 | cache_unfriendly | 47393364.93 | 58.62 | 121.24 | 131.08 | 0.1042 | 40 | cache_locality_pressure |
| single_threaded | 1 | 100000 | cache_local | 35460992.91 | 7.21 | 7.29 | 7.38 | 0.0000 | 391 | allocation_pressure |
| multithreaded | 1 | 100000 | cache_local | 35520824.08 | 1405.42 | 2614.00 | 2720.06 | 0.0026 | 391 | queue_wait_pressure |
| multithreaded | 2 | 100000 | cache_local | 68890260.57 | 724.71 | 1321.75 | 1373.03 | 0.0254 | 391 | queue_wait_pressure |
| multithreaded | 4 | 100000 | cache_local | 111898503.58 | 364.17 | 592.31 | 608.96 | 0.0532 | 391 | lock_contention |
| multithreaded | 8 | 100000 | cache_local | 102577253.49 | 233.50 | 401.60 | 425.53 | 0.1729 | 391 | lock_contention |
| single_threaded | 1 | 100000 | cache_unfriendly | 32432435.94 | 7.79 | 7.88 | 7.98 | 0.0000 | 391 | cache_locality_pressure |
| multithreaded | 1 | 100000 | cache_unfriendly | 31189891.23 | 1600.79 | 2999.90 | 3123.87 | 0.0026 | 391 | cache_locality_pressure |
| multithreaded | 2 | 100000 | cache_unfriendly | 58633831.72 | 846.38 | 1567.98 | 1632.51 | 0.0000 | 391 | cache_locality_pressure |
| multithreaded | 4 | 100000 | cache_unfriendly | 100217070.17 | 472.54 | 794.33 | 826.38 | 0.0405 | 391 | cache_locality_pressure |
| multithreaded | 8 | 100000 | cache_unfriendly | 94276652.95 | 437.46 | 752.46 | 778.48 | 0.1704 | 391 | cache_locality_pressure |

## Metrics

- throughput measures processed data items per second
- task latency includes queue wait plus task execution
- lock contention counts failed immediate mutex acquisitions
- allocation counts represent benchmark-owned temporary buffers
- cache-local and cache-unfriendly cases process equivalent data

## Claims boundary

This benchmark uses deterministic synthetic workloads. It does not represent production trading, exchange, market-data, or Citadel internal infrastructure.
