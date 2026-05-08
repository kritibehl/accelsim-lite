# Memory-Port What-If Sweep

| Memory Ports | Throughput | Avg Latency | Top Bottleneck | NoMemoryPort Stalls |
|---:|---:|---:|---|---:|
| 1 | 0.1395 | 23.8333 | NoMemoryPort | 49 |
| 2 | 0.2222 | 16.3333 | WaitingDependency | 11 |
| 4 | 0.2609 | 14.5000 | WaitingDependency | 0 |

## 1 → 4 memory ports

- Latency improvement: `39.16%`
- Throughput improvement: `87.03%`
- Bottleneck shift: `NoMemoryPort -> WaitingDependency`
- NoMemoryPort stall reduction: `49`
