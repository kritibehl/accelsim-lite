# Communication Bottleneck Report

> Scope: synthetic workload-level communication pressure analysis, not a real hardware collective benchmark.

| Scenario | Groups | Throughput | Total Cycles | Transfer Cycles | Queue Delay | Comm Ratio | Queue Ratio | Bottleneck |
|---|---:|---:|---:|---:|---:|---:|---:|---|
| compute_heavy | 1 | 0.007576 | 132 | 8 | 4 | 0.0606 | 0.0303 | compute-limited |
| compute_heavy | 2 | 0.013158 | 152 | 16 | 16 | 0.1053 | 0.1053 | compute-limited |
| compute_heavy | 4 | 0.018519 | 216 | 32 | 64 | 0.1481 | 0.2963 | queue-pressure-limited |
| transfer_heavy_all_gather_style | 1 | 0.007692 | 130 | 42 | 8 | 0.3231 | 0.0615 | memory-port-limited |
| transfer_heavy_all_gather_style | 2 | 0.010204 | 196 | 84 | 32 | 0.4286 | 0.1633 | communication-pressure-limited |
| transfer_heavy_all_gather_style | 4 | 0.010638 | 376 | 168 | 128 | 0.4468 | 0.3404 | communication-pressure-limited |
| reduction_style_pressure | 1 | 0.007692 | 130 | 28 | 6 | 0.2154 | 0.0462 | compute-limited |
| reduction_style_pressure | 2 | 0.011364 | 176 | 56 | 24 | 0.3182 | 0.1364 | memory-port-limited |
| reduction_style_pressure | 4 | 0.013158 | 304 | 112 | 96 | 0.3684 | 0.3158 | communication-pressure-limited |

## Key observations

- Compute-heavy workloads remain mostly compute-limited as simulated groups increase.
- Transfer-heavy all-gather-style workloads shift toward communication-pressure limits.
- Queue delay grows superlinearly in this synthetic model as more simulated compute groups contend for transfer capacity.
- Reduction-style pressure shows an intermediate regime where communication and queue delay both influence throughput.
- This report is intended for workload-level bottleneck reasoning, not real hardware timing prediction.

## Safe interpretation

AccelSim-Lite uses this experiment to compare compute-heavy and transfer-heavy execution patterns under synthetic communication pressure. It surfaces utilization shifts, queue-delay growth, and bottleneck classifications that are useful for reasoning about accelerator-style workload balance.

