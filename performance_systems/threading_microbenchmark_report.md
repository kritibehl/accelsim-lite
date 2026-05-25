# Threading / Queue Concurrency Microbenchmark

> Scope: queue-worker concurrency microbenchmark. This is not a production scheduler or lock-free systems benchmark.

| Workers | Tasks | Elapsed ms | Throughput tasks/sec | Relative throughput |
|---:|---:|---:|---:|---:|
| 1 | 20000 | 44.7496 | 446931.54 | 1.0x |
| 2 | 20000 | 44.0662 | 453862.52 | 1.0155x |
| 4 | 20000 | 48.5244 | 412163.99 | 0.9222x |
| 8 | 20000 | 47.1319 | 424340.9 | 0.9495x |

## What this means

This experiment models a shared task-queue worker pattern and reports how throughput changes as worker count increases.

It is useful for discussing:
- queue contention
- worker scaling
- shared work distribution
- concurrency overhead
- throughput measurement methodology

## Claims boundary

This is an educational concurrency microbenchmark. It does not claim production thread-pool engineering, lock-free queue implementation, or HFT-grade latency measurement.
