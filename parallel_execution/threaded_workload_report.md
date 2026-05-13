# Threaded Workload Report

> Scope: std::thread workload scaling experiment with deterministic simulated-cycle accounting. This is not a hardware timing benchmark.

| Scenario | Workers | Tasks | Max Worker Cycles | Imbalance | Throughput | Bottleneck |
|---|---:|---:|---:|---:|---:|---|
| balanced_partition | 1 | 48 | 600 | 0 | 0.080000 | serial-bound |
| imbalanced_partition | 1 | 48 | 586 | 0 | 0.081911 | serial-bound |
| balanced_partition | 2 | 48 | 600 | 600 | 0.080000 | worker-imbalance-limited |
| imbalanced_partition | 2 | 48 | 586 | 586 | 0.081911 | worker-imbalance-limited |
| balanced_partition | 4 | 48 | 600 | 600 | 0.080000 | worker-imbalance-limited |
| imbalanced_partition | 4 | 48 | 586 | 586 | 0.081911 | worker-imbalance-limited |

## Key observations

- Worker scaling improves throughput when work is balanced and contention is low.
- Imbalanced partitioning can limit parallel speedup even when more worker threads are available.
- Transfer-heavy tasks increase contention pressure and reduce effective parallel scaling.
- Bottleneck classifications are workload-level labels, not hardware performance claims.

## Safe interpretation

This experiment demonstrates C++ threading, shared-queue coordination, worker-level accounting, and bottleneck classification for synthetic accelerator-style workloads.
