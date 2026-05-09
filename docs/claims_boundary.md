# AccelSim-Lite Claims Boundary

AccelSim-Lite is a deterministic workload-level accelerator pipeline simulator.

It is designed for:
- workload-level throughput analysis
- workload-level latency analysis
- bottleneck classification
- memory-vs-dependency pressure analysis
- what-if resource sensitivity experiments

## Safe claims

It is safe to claim that AccelSim-Lite:

- models throughput and latency across synthetic workload classes
- compares compute-heavy, memory-heavy, mixed, and queue-pressure workloads
- classifies bottlenecks such as `NoMemoryPort`, `WaitingDependency`, and `NoComputeUnit`
- performs memory-port what-if sweeps
- generates workload comparison reports and dashboard artifacts
- helps reason about high-level accelerator-style performance trends

## Do-not-claim list

Do NOT claim AccelSim-Lite proves:

- cycle-accurate GPU simulation
- CUDA kernel execution
- Tensor Core simulation
- warp scheduling behavior
- cache-line behavior
- SM-level occupancy modeling
- hardware-accurate NVIDIA/A100/H100 latency prediction
- real MLPerf benchmarking
- production GPU runtime optimization
- production LLM inference optimization
- distributed systems scalability
- real hardware validation

## Interview-safe wording

Use this:

> AccelSim-Lite is not a cycle-accurate GPU simulator. It is a deterministic workload-level performance model that helps reason about throughput, latency, and bottleneck shifts under constrained compute and memory resources.

Avoid this:

> Built an accurate GPU simulator.
> Matched NVIDIA hardware performance.
> Simulated CUDA/Tensor Core execution.
> Optimized real LLM inference workloads.
