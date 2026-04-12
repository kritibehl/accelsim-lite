<div align="center">

# AccelSim-Lite

**Deterministic accelerator pipeline simulator for workload-level performance analysis**

[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)](https://isocpp.org)
[![CMake](https://img.shields.io/badge/CMake-Build-064F8C?style=flat-square&logo=cmake&logoColor=white)](https://cmake.org)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

</div>

---

> Performance profilers tell you a workload is slow.
> **AccelSim-Lite tells you *why* — which pipeline stage is the binding constraint, and how that changes across workload types.**

---

## What it models

AccelSim-Lite simulates a six-stage compute pipeline:

```
Fetch → Decode → Dispatch → Issue → Execute → Retire
```

For each workload it measures:

- **Throughput** (ops/cycle) — how many operations complete per clock cycle
- **Latency** (cycles/op) — average cycles from fetch to retire
- **Stall classification** — which stage is the binding constraint
- **Queue occupancy** — pressure across all six pipeline stages
- **Bottleneck transitions** — how the dominant constraint shifts across workload types

---

## Validated results

All four workload profiles produce deterministic, reproducible output:

| Workload | Throughput (ops/cycle) | Avg latency (cycles) | Dominant bottleneck |
|---|---|---|---|
| `compute_heavy` | 0.3333 | 10.0 | WaitingDependency |
| `memory_heavy` | 0.1395 | 23.8 | NoMemoryPort |
| `mixed` | 0.20 | 16.8 | WaitingDependency |
| `queue_pressure` | 0.32 | 15.0 | WaitingDependency + NoComputeUnit |

### Key findings

**Memory pressure causes ~2.4× throughput degradation and ~2.4× latency increase** compared to compute-heavy execution. The binding constraint shifts from instruction dependency (WaitingDependency) to memory bandwidth (NoMemoryPort) — reflecting real accelerator behavior where data movement becomes the bottleneck before compute capacity is exhausted.

**Queue-pressure workloads surface compute contention** (NoComputeUnit=24) alongside dependency stalls, exposing the interaction between scheduling pressure and execution unit availability under mixed load.

**Bottleneck transitions are workload-driven, not configuration-driven.** The same pipeline configuration behaves differently depending on instruction mix and access pattern — which is the core insight AccelSim-Lite makes observable.

---

## Connection to LLM inference

The memory-bandwidth bottleneck modeled in `memory_heavy` maps directly to transformer inference behavior:

- KV-cache access during autoregressive decoding is bandwidth-limited, not compute-limited
- Attention computation transitions from compute-bound (small batches) to memory-bound (large batches)
- The `NoMemoryPort` stall pattern in AccelSim-Lite models this bandwidth saturation

This makes the project relevant to anyone reasoning about inference optimization, hardware selection for LLM serving, or the compute/memory trade-off in AI workload scheduling.

---

## Output artifacts

Every run produces structured reports under `reports/latest/`:

| Artifact | Contents |
|---|---|
| `summary.json` | Throughput, latency, dominant bottleneck per workload |
| `stalls.csv` | Per-stage stall counts |
| `utilization.csv` | Stage busy-cycle fractions |
| `queue_occupancy.csv` | Max queue occupancy across stages |
| `pipeline_utilization.png` | Visual stage utilization breakdown |
| `stall_breakdown.png` | Stall distribution by type |

---

## Scope

AccelSim-Lite is a **workload-level performance model**, not a cycle-accurate GPU simulator.

What it captures faithfully:
- Throughput and latency trends across workload types
- Relative bottleneck behavior: dependency vs memory vs compute vs queue
- Stall classification and pipeline occupancy patterns

What it deliberately simplifies:
- No SIMD or warp-level execution
- No cache hierarchy (L1/L2/shared memory)
- No inter-SM communication or memory controller modeling
- No instruction-level cycle accuracy

The goal is bottleneck reasoning and capacity analysis, not hardware verification.

---

## Running

```bash
# Build
cmake -B build && cmake --build build

# Run a workload
./accelsim-lite run workloads/compute_heavy.csv

# Run all workloads
./accelsim-lite benchmark all

# View results
./accelsim-lite report latest
cat reports/latest/summary.json
```

---

## What-if analysis

AccelSim-Lite supports parameter sweeps for capacity reasoning:

- What throughput do I get if I double compute units?
- How does reducing queue depth affect latency under queue-pressure workloads?
- At what memory bandwidth does a workload transition from compute-bound to memory-bound?

---

## Interview framing

AccelSim-Lite started as a question: can I build a performance model that explains *why* a workload is slow, not just *that* it is slow? The stall classification system — WaitingDependency, NoMemoryPort, NoComputeUnit — gives a named reason for every cycle that does not produce output. That named classification is what makes it useful for capacity reasoning rather than just measurement.

---

## Stack

C++ · CMake

---

## Related

- [DetTrace](https://github.com/kritibehl/dettrace) — deterministic replay for concurrency failures in software systems
- [KubePulse](https://github.com/kritibehl/KubePulse) — performance and resilience validation for Kubernetes services
- [Faultline](https://github.com/kritibehl/faultline) — correctness under failure for distributed job execution
