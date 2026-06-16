# Scheduler Regression Report

Scheduler validation covers FIFO/round-robin/priority-style workload behavior, queue pressure, worker contention, and latency tradeoffs.

## Regression signals

- increased queue wait time
- throughput drop under worker scaling
- contention-limited behavior
- scheduler fairness drift
