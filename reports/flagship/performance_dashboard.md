# AccelSim-Lite Performance Dashboard

## Core simulator proof points

| Area | Verified signal |
|---|---|
| Memory-port scaling | 1 -> 4 memory ports improved memory-heavy latency by 39.16% |
| Throughput scaling | 1 -> 4 memory ports improved throughput by 87.03% |
| Bottleneck transition | NoMemoryPort -> WaitingDependency |
| Runtime equivalence | 6/6 semantic matches, 0 semantic mismatches |
| Wasm malformed traces | 7/7 malformed traces correctly rejected |
| Runtime regression gate | PASS |
| ML systems study | prefill/decode bottleneck transitions reported |
| Metadata lookup | hash-table vs tree-backed lookup benchmark generated |

## Scope

Metrics are simulator-derived and intended for systems/performance reasoning, not hardware benchmarking.
