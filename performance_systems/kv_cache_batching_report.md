# KV-Cache and Token Batching Simulation

> Scope: simulated inference-systems pressure model. Not real LLM serving or accelerator benchmarking.

| Profile | Latency improvement 1->8 ports | Throughput gain 1->8 ports | Bottleneck @1 port | Bottleneck @8 ports |
|---|---:|---:|---|---|
| small_batch_decode | 0.28% | 0.28% | decode-dependency-pressure | decode-dependency-pressure |
| medium_batch_decode | 1.81% | 1.84% | decode-dependency-pressure | decode-dependency-pressure |
| large_batch_prefill_decode | 3.55% | 3.68% | decode-dependency-pressure | decode-dependency-pressure |
| queue_pressure_serving | 3.44% | 3.57% | decode-dependency-pressure | decode-dependency-pressure |

## Interpretation

- Larger contexts and batches increase simulated KV-cache pressure.
- Memory-port scaling reduces KV pressure, after which decode dependency can dominate.
- Queue-pressure serving cases show batching overhead as a separate serving constraint.

## Claims boundary

This is a synthetic inference-systems model, not a real LLM serving benchmark.
