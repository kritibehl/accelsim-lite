# Cache Hierarchy Behavior Study

> Scope: simulated cache hierarchy behavior. This is not hardware-counter measurement, CPU microarchitecture profiling, or production performance benchmarking.

## Latency model

| Access level | Simulated latency |
|---|---:|
| L1 hit | 4 cycles |
| L2 hit | 12 cycles |
| Memory access | 120 cycles |

## Results

| Profile | L1 hit | L2 hit | Memory access | Avg latency cycles | Slowdown vs all-L1 |
|---|---:|---:|---:|---:|---:|
| hot_working_set | 86% | 10% | 4% | 9.44 | 2.36x |
| mixed_locality | 55% | 30% | 15% | 23.8 | 5.95x |
| streaming_access | 22% | 28% | 50% | 64.24 | 16.06x |
| pointer_chasing | 12% | 18% | 70% | 86.64 | 21.66x |

## What this means

- Hot working sets stay closer to L1 behavior and keep average access latency low.
- Mixed-locality workloads become more sensitive to L2 and memory behavior.
- Streaming and pointer-chasing patterns experience much higher average latency because more accesses fall through to memory.
- This demonstrates cache hierarchy reasoning, memory-locality tradeoffs, and latency impact analysis for systems-performance discussion.

## Claims boundary

This is a simulated cache hierarchy study. It does not claim real hardware cache measurement, perf-counter analysis, or CPU microarchitecture benchmarking.
