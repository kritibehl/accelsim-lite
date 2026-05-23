# Threading and Contention Notes

## Current threading signals

AccelSim-Lite includes:
- `parallel_execution/threaded_workload_runner.cpp`
- worker-scaling reports
- queue-pressure experiments
- scheduler experiments
- runtime regression gates

## Concepts covered

- worker-count scaling
- queue pressure under parallel execution
- balanced vs imbalanced synthetic workloads
- contention-limited behavior
- serial-bound vs worker-imbalance-limited classification

## Future extension

A future deeper implementation could add:
- reusable thread pool
- lock contention counters
- work stealing queue
- mutex vs atomic queue comparison
- per-worker latency histograms

## Claims boundary

The current implementation demonstrates concurrency experimentation and threaded workload modeling. It does not claim production scheduler implementation or lock-free systems engineering.
