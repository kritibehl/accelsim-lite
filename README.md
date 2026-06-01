# AccelSim-Lite

> A production-style C++ runtime and performance validation platform that identifies the binding bottleneck in a compute pipeline and gates regressions with ASan, UBSan, and GoogleTest CI.

`C++17` · `CMake` · `GoogleTest` · `Python`

[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus)](https://isocpp.org)
[![CMake](https://img.shields.io/badge/CMake-Build-064F8C?style=flat-square&logo=cmake)](https://cmake.org)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

---

## Why This Project Matters

- Performance profilers tell you a workload is slow; they don't tell you which pipeline stage is the constraint, or how that changes as the workload shifts
- The bottleneck moves: compute-heavy workloads stall on instruction dependencies; memory-heavy workloads stall on bandwidth; choosing hardware or sizing capacity requires knowing which one you're hitting
- AccelSim makes the bottleneck explicit — every cycle that doesn't produce output has a named reason
- This proves: C++ systems engineering, performance analysis discipline, benchmark automation, and CI quality gate design

---

## 30-Second Proof

| Signal | Verified output |
|---|---|
| p95 regression detected | **+51.22%** → gated |
| Optimization speedup | **3.64×** |
| Memory pressure throughput regression | **2.4× vs compute-heavy baseline** |
| GoogleTest suite | **32 tests passing** |
| AddressSanitizer | **PASS** — 0 heap errors |
| UndefinedBehaviorSanitizer | **PASS** — 0 undefined behavior |
| Warnings-as-errors | **PASS** |

---

## Runtime Advisor

After every benchmark run, AccelSim produces a recommendation based on the dominant stall:

```json
{
  "workload": "memory_heavy",
  "bottleneck": "NoMemoryPort",
  "throughput": 0.140,
  "recommendation": "Workload is bandwidth-bound. Adding compute units will not improve throughput. Reduce memory access frequency or increase data locality.",
  "action": "restructure_data_layout"
}
```

```json
{
  "workload": "compute_heavy",
  "bottleneck": "WaitingDependency",
  "throughput": 0.333,
  "recommendation": "Workload is instruction-dependency-bound. Reorder instructions to reduce dependency chains or increase instruction-level parallelism.",
  "action": "reorder_instruction_schedule"
}
```

---

## Optimization Explorer

Baseline vs optimized comparison — 3.64× speedup measured and logged:

```
┌─────────────────────────────────────────────────────────────┐
│  Optimization Explorer — compute_heavy workload             │
├────────────────────┬────────────────┬───────────────────────┤
│  Version           │  Throughput    │  Dominant stall       │
├────────────────────┼────────────────┼───────────────────────┤
│  baseline          │  0.091 ops/cyc │  WaitingDependency    │
│  optimized         │  0.333 ops/cyc │  WaitingDependency    │
│  speedup           │  3.64×         │                       │
├────────────────────┴────────────────┴───────────────────────┤
│  p95 regression gate:  +51.22% detected on prior run        │
│  Current run:          within threshold  ✓                  │
│  ASan:  PASS  ·  UBSan:  PASS  ·  Warnings:  PASS           │
└─────────────────────────────────────────────────────────────┘
```

```bash
./accelsim-lite compare --baseline v1 --candidate v2
# → speedup: 3.64x · bottleneck unchanged: WaitingDependency
```

---

## Demo

```bash
git clone https://github.com/kritibehl/accelsim-lite
cd accelsim-lite
make demo
```

Expected output:
```json
{
  "compute_heavy":  { "throughput": 0.333, "latency": 10.0, "bottleneck": "WaitingDependency" },
  "memory_heavy":   { "throughput": 0.140, "latency": 23.8, "bottleneck": "NoMemoryPort" },
  "mixed":          { "throughput": 0.200, "latency": 16.8, "bottleneck": "WaitingDependency" },
  "queue_pressure": { "throughput": 0.320, "latency": 15.0, "bottleneck": "WaitingDependency+NoComputeUnit" },
  "memory_regression_vs_compute": "2.4x throughput degradation",
  "p95_regression_detected": true,
  "regression_pct": 51.22
}
```

```bash
make test    # 32 GoogleTest cases · ASan · UBSan · 0 failures
make report  # benchmark report → reports/latest/summary.json
```

---

## Architecture

```
Workload definition (CSV)
  instruction mix · memory access rate · batch size
      │
      ▼
Six-stage pipeline simulator (C++17)
  Fetch → Decode → Dispatch → Issue → Execute → Retire
      │
      ▼
Per-cycle stall classifier
  ├── WaitingDependency   instruction dependency not resolved
  ├── NoMemoryPort        memory bandwidth exhausted
  └── NoComputeUnit       execution unit not available
      │
      ▼
Benchmark reporter
  throughput (ops/cycle) · latency (cycles/op)
  stage utilization · queue occupancy · stall breakdown
      │
      ▼
Regression gate
  p95 latency delta > threshold → FAIL
  throughput drop > threshold   → FAIL
      │
      ▼
CI quality gate (CMake + ASan + UBSan + warnings-as-errors)
```

---

## Core Workflows

### 1. Bottleneck identification

Runs all 4 workload profiles, classifies dominant stall per cycle, reports which constraint is binding and by how much.

```bash
make demo
# memory_heavy: NoMemoryPort · 2.4x regression vs compute_heavy
```

### 2. Regression gate

Detects p95 latency regressions across benchmark runs. Gates on configurable threshold — blocks if exceeded.

```
p95 regression detected: +51.22%  →  GATE FAILS
optimization speedup:    3.64x    →  candidate approved
```

### 3. CI quality gate

Full build with AddressSanitizer, UndefinedBehaviorSanitizer, and warnings-as-errors. 0 errors required to pass.

```bash
make test
# 32 tests · ASan: PASS · UBSan: PASS · warnings: PASS
```

---

## Failure Scenarios Covered

| Workload | Dominant bottleneck | Key finding |
|---|---|---|
| `compute_heavy` | WaitingDependency | 10.0 cycles/op baseline |
| `memory_heavy` | **NoMemoryPort** | 2.4× throughput regression — bandwidth-bound |
| `mixed` | WaitingDependency | Instruction dep dominates over memory |
| `queue_pressure` | WaitingDependency + NoComputeUnit | Both stall types active simultaneously |
| p95 regression | Latency spike | +51.22% detected and gated |
| Optimization | Speedup measurement | 3.64× confirmed |

---

## Engineering Decisions

**Why named stall classification instead of a single throughput number:** A throughput number tells you the workload is slow. A named stall tells you *why* — and whether adding compute units, memory bandwidth, or queue capacity will help. These are different fixes.

**Why 6 pipeline stages:** Fetch/Decode/Dispatch/Issue/Execute/Retire mirrors real pipeline structure. Stalls at Dispatch indicate queue pressure; stalls at Execute indicate compute or memory constraints. Stage granularity makes the diagnosis specific.

**Why ASan + UBSan in CI:** Performance benchmarks can mask memory errors — a stale pointer might not crash but produces wrong measurements. ASan/UBSan run on every build ensures benchmark outputs are from valid execution, not undefined behavior.

---

## What Is Intentionally Out of Scope

- Workload-level performance model, not a cycle-accurate GPU simulator
- No SIMD, warp-level execution, or cache hierarchy (L1/L2/shared memory)
- Memory bottleneck pattern is analogous to KV-cache pressure — not GPU hardware modeling
- No instruction-level cycle accuracy — goal is bottleneck reasoning, not hardware verification

---

## Resume Bullets

- Built a C++17 six-stage pipeline simulator with named per-cycle stall classification (WaitingDependency / NoMemoryPort / NoComputeUnit), validated a 2.4× throughput regression at the memory bandwidth boundary
- Implemented a p95 regression gate catching a +51.22% latency regression and a 3.64× optimization speedup across 4 deterministic workload profiles
- Full CI quality gate: CMake build, 32 GoogleTest cases, AddressSanitizer clean, UndefinedBehaviorSanitizer clean, warnings-as-errors

---

## Interview Walkthrough

*"AccelSim answers a specific question: which stage of the pipeline is the bottleneck, and how does that change as the workload shifts? I built a 6-stage C++ simulator and classify every idle cycle with a named reason — WaitingDependency, NoMemoryPort, or NoComputeUnit. Memory-heavy workloads show a 2.4× throughput regression versus compute-heavy, and the named stall tells you that adding compute units won't help — it's bandwidth-bound. I also have a p95 regression gate that caught a +51.22% regression, and a full CI pipeline with ASan, UBSan, and warnings-as-errors."*

---

## Run Locally

```bash
git clone https://github.com/kritibehl/accelsim-lite && cd accelsim-lite
cmake -B build && cmake --build build
make demo    # all 4 workloads + regression gate
make test    # 32 GoogleTest · ASan · UBSan
make report  # benchmark report → reports/latest/
```

---

## Repository Map

```
accelsim-lite/
├── src/          C++17 pipeline simulator + stall classifier
├── workloads/    CSV workload definitions
├── bench/        Benchmark runner + regression gate
├── reports/      Benchmark outputs + regression reports
└── docs/         Architecture + perf charts
```
