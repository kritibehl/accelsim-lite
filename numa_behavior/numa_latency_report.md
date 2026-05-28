# Simulated NUMA Local vs Remote Memory Study

> Scope: simulated NUMA-locality study. This is not hardware NUMA profiling or production performance benchmarking.

## Latency model

| Access type | Simulated latency |
|---|---:|
| Local memory | 80 cycles |
| Remote memory | 155 cycles |

## Results

| Profile | Local access | Remote access | Avg latency | Delta vs all-local | Slowdown |
|---|---:|---:|---:|---:|---:|
| local_memory_access | 92% | 8% | 86.0 cycles | 6.0 cycles | 1.075x |
| balanced_cross_socket_access | 60% | 40% | 110.0 cycles | 30.0 cycles | 1.375x |
| remote_heavy_access | 30% | 70% | 132.5 cycles | 52.5 cycles | 1.656x |

## What this means

- Local memory access keeps latency closer to the baseline local-memory model.
- Cross-socket or remote-heavy access increases average memory latency.
- This demonstrates locality-aware systems reasoning, memory-placement tradeoffs, and latency-impact analysis.

## Claims boundary

This is a simulated NUMA-locality study. It does not claim real NUMA hardware measurement, OS scheduler implementation, or CPU performance-counter profiling.
