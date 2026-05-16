# LLM Inference Bottleneck Study

> Scope: simulated transformer-style inference workload study. This is not a real LLM serving benchmark, accelerator benchmark, or hardware measurement.

| Workload | Latency improvement 1->8 memory ports | Throughput gain 1->8 memory ports | Baseline bottleneck | Scaled bottleneck |
|---|---:|---:|---|---|
| prefill_heavy | 36.59% | 57.7% | memory-bound | queue-pressure-bound |
| decode_heavy | 27.78% | 38.46% | dependency-bound | dependency-bound |
| mixed_serving_trace | 32.79% | 48.78% | memory-bound | dependency-bound |
| queue_pressure_serving_trace | 33.08% | 49.44% | memory-bound | dependency-bound |

## Key observations

- Prefill-heavy traces are more sensitive to memory-port scaling because they model wider parallel compute and memory demand.
- Decode-heavy traces remain more dependency-sensitive because sequential token-generation pressure dominates after memory stalls shrink.
- Mixed serving traces show combined memory and dependency pressure.
- Queue-pressure serving traces surface scheduling/queue delay as concurrency increases.

## Safe interpretation

This study uses simulated workload profiles to reason about bottleneck transitions in transformer-style inference phases. It does not claim real model inference, real accelerator performance, or production serving measurements.
