# Workload Comparison Report

| Workload | Cycles | Ops | Throughput | Avg Latency | Top Bottleneck |
|---|---:|---:|---:|---:|---|
| compute_heavy | 18 | 6 | 0.3333 | 10.0000 | WaitingDependency |
| memory_heavy | 43 | 6 | 0.1395 | 23.8333 | NoMemoryPort |
| mixed | 30 | 6 | 0.2000 | 16.8333 | WaitingDependency |
| queue_pressure | 25 | 8 | 0.3200 | 15.0000 | WaitingDependency |

## Derived metrics

- Throughput range: `0.1395--0.3333 ops/cycle`
- Latency range: `10.0--23.8333 cycles`
- Memory-heavy throughput drop vs compute-heavy: `58.15%`
- Memory-heavy latency increase vs compute-heavy: `2.38x`
