# Scheduler Trace Report

> Scope: synthetic workload scheduling experiment comparing FIFO, round-robin, and priority scheduling. This is not an OS scheduler benchmark.

| Policy | Total Time | Avg Wait | Avg Turnaround | Throughput | Max Wait | Fairness Index | Starvation Risk |
|---|---:|---:|---:|---:|---:|---:|---|
| fifo | 66 | 26.00 | 34.25 | 0.121212 | 43 | 0.7669 | elevated |
| round_robin_q4 | 66 | 29.75 | 38.00 | 0.121212 | 44 | 0.7888 | elevated |
| priority | 66 | 20.12 | 28.38 | 0.121212 | 43 | 0.7009 | elevated |

## Scheduling traces

### fifo

| Task | Kind | Priority | Arrival | Start | Finish | Wait | Turnaround |
|---|---|---:|---:|---:|---:|---:|---:|
| cpu-render-0 | cpu_bound | 2 | 0 | 0 | 14 | 0 | 14 |
| io-fetch-0 | io_bound | 1 | 1 | 14 | 18 | 13 | 17 |
| cpu-compile-1 | cpu_bound | 3 | 2 | 18 | 30 | 16 | 28 |
| io-fetch-1 | io_bound | 1 | 3 | 30 | 33 | 27 | 30 |
| cpu-analysis-2 | cpu_bound | 2 | 4 | 33 | 43 | 29 | 39 |
| io-sync-2 | io_bound | 1 | 5 | 43 | 48 | 38 | 43 |
| priority-control | io_bound | 0 | 6 | 48 | 50 | 42 | 44 |
| cpu-batch-3 | cpu_bound | 4 | 7 | 50 | 66 | 43 | 59 |

### round_robin_q4

| Task | Kind | Priority | Arrival | Start | Finish | Wait | Turnaround |
|---|---|---:|---:|---:|---:|---:|---:|
| cpu-render-0 | cpu_bound | 2 | 0 | 0 | 58 | 44 | 58 |
| io-fetch-0 | io_bound | 1 | 1 | 4 | 8 | 3 | 7 |
| cpu-compile-1 | cpu_bound | 3 | 2 | 8 | 54 | 40 | 52 |
| io-fetch-1 | io_bound | 1 | 3 | 12 | 15 | 9 | 12 |
| cpu-analysis-2 | cpu_bound | 2 | 4 | 15 | 56 | 42 | 52 |
| io-sync-2 | io_bound | 1 | 5 | 23 | 46 | 36 | 41 |
| priority-control | io_bound | 0 | 6 | 27 | 29 | 21 | 23 |
| cpu-batch-3 | cpu_bound | 4 | 7 | 29 | 66 | 43 | 59 |

### priority

| Task | Kind | Priority | Arrival | Start | Finish | Wait | Turnaround |
|---|---|---:|---:|---:|---:|---:|---:|
| cpu-render-0 | cpu_bound | 2 | 0 | 0 | 14 | 0 | 14 |
| priority-control | io_bound | 0 | 6 | 14 | 16 | 8 | 10 |
| io-fetch-0 | io_bound | 1 | 1 | 16 | 20 | 15 | 19 |
| io-fetch-1 | io_bound | 1 | 3 | 20 | 23 | 17 | 20 |
| io-sync-2 | io_bound | 1 | 5 | 23 | 28 | 18 | 23 |
| cpu-analysis-2 | cpu_bound | 2 | 4 | 28 | 38 | 24 | 34 |
| cpu-compile-1 | cpu_bound | 3 | 2 | 38 | 50 | 36 | 48 |
| cpu-batch-3 | cpu_bound | 4 | 7 | 50 | 66 | 43 | 59 |

## Interpretation

- FIFO preserves arrival order but can increase wait time for short I/O-bound tasks behind long CPU-bound tasks.
- Round-robin improves response distribution but can increase turnaround for CPU-heavy tasks depending on quantum size.
- Priority scheduling improves latency for high-priority I/O/control tasks but can increase starvation risk for lower-priority CPU-bound tasks.
- The experiment is intended for workload-management reasoning, not OS-kernel scheduler benchmarking.
