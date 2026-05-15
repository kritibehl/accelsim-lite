# accelsim-lite

> A deterministic C++ compute-pipeline simulator for analyzing workload latency, throughput, queue pressure, and bottleneck behavior under configurable resource constraints.

---

## What it is

**accelsim-lite** is a deterministic C++ workload simulator for comparative performance analysis. It models a staged execution pipeline, enforces configurable compute and memory resource limits, tracks dependency and resource stalls, and exports structured reports for benchmarking, diffing, and charting.

It is not a full ISA or cycle-accurate hardware simulator. It is a focused systems-performance tool for understanding how different workload classes behave under constrained execution resources.

---

## Pipeline model

Every instruction moves through seven stages in order:

```
Fetch → Decode → Dispatch → Ready → Execute → Retire → Done
```

| Stage      | Description                                                  |
|------------|--------------------------------------------------------------|
| `Fetch`    | Instruction enters the pipeline                              |
| `Decode`   | Instruction advances through decode bandwidth limits              |
| `Dispatch` | Instruction queued for issue (bounded by `dispatch_queue_capacity`) |
| `Ready`    | All dependencies retired; instruction queued for execution   |
| `Execute`  | Resource consumed for `latency` cycles                       |
| `Retire`   | Instruction completes; dependents unblocked                  |
| `Done`     | Instruction recorded in the completed trace                  |

---

## Resource limits

Configure the simulator by adjusting these four parameters:

| Parameter                | Controls                                      |
|--------------------------|-----------------------------------------------|
| `compute_units`          | Max concurrent COMPUTE ops                    |
| `memory_ports`           | Max concurrent LOAD/STORE ops                 |
| `dispatch_queue_capacity`| Max instructions waiting to be issued         |
| `ready_queue_capacity`   | Max instructions with resolved dependencies   |

---

## Op types

| Type      | Uses              |
|-----------|-------------------|
| `COMPUTE` | Compute units     |
| `LOAD`    | Memory ports      |
| `STORE`   | Memory ports      |

---

## Input format

Workloads are CSV files placed in `workloads/`. Each row is one instruction.

### Fields

| Field          | Type     | Description                                      |
|----------------|----------|--------------------------------------------------|
| `id`           | integer  | Unique instruction identifier                    |
| `op_type`      | string   | `COMPUTE`, `LOAD`, or `STORE`                    |
| `latency`      | integer  | Execution cycles                                 |
| `memory_bytes` | integer  | Bytes accessed (LOAD/STORE only; ignored otherwise) |
| `deps`         | string   | `\|`- or `;`-separated list of instruction IDs this op depends on (empty = no deps) |

### Example

```csv
id,op_type,latency,memory_bytes,deps
1,COMPUTE,4,0,
2,LOAD,8,64,1
3,COMPUTE,2,0,2
4,STORE,6,64,3
```

### Parser validation

The parser rejects workloads with:
- duplicate instruction IDs
- references to dependency IDs that do not exist
- unrecognized op types

Empty `deps` fields are valid and treated as no dependencies.

---

## Metrics collected

Each run produces the following metrics:

| Metric                | Description                                             |
|-----------------------|---------------------------------------------------------|
| `total_cycles`        | Cycles from first fetch to last retire                  |
| `completed_ops`       | Number of instructions that reached `Done`              |
| `throughput`          | Ops per cycle                                           |
| `average_latency`     | Mean cycles per instruction (fetch → retire)            |
| `stage_busy_cycles`   | Per-stage cycle counts where the stage was active       |
| `stall_counts`        | Per-stall-type counts (see below)                       |
| `max_queue_occupancy` | Peak depth recorded for dispatch and ready queues       |
| `top_bottleneck`      | Stall type with the highest count for this run          |

---

## Stall and bottleneck analysis

The simulator accounts for five stall categories:

| Stall type          | Triggered when                                              |
|---------------------|-------------------------------------------------------------|
| `DispatchQueueFull` | Dispatch queue is at capacity; fetch stalls                 |
| `ReadyQueueFull`    | Ready queue is at capacity; dispatch stalls                 |
| `WaitingDependency` | Instruction has unretired dependencies; cannot enter Ready  |
| `NoComputeUnit`     | All compute units occupied; COMPUTE op stalls in Ready      |
| `NoMemoryPort`      | All memory ports occupied; LOAD/STORE op stalls in Ready    |

`top_bottleneck` in the summary identifies which stall type dominated the run.

---

## Reports

Every run exports to `reports/latest/`:

| File                   | Contents                                              |
|------------------------|-------------------------------------------------------|
| `summary.txt`          | Human-readable metrics summary                        |
| `summary.json`         | Machine-readable metrics (for scripting/CI)           |
| `utilization.csv`      | Per-stage busy cycle counts                           |
| `stalls.csv`           | Per-stall-type counts                                 |
| `queue_occupancy.csv`  | Peak occupancy recorded for tracked queues                    |
| `completed_trace.csv`  | Per-instruction enqueue, issue, and completion cycles         |

---

## CLI

```sh
# Run a single workload and export a report
accelsim-lite run <workload.csv>

# Run two workloads and produce a diff summary
accelsim-lite compare <left.csv> <right.csv>

# Run all bundled workloads
accelsim-lite benchmark all

# Print the most recently saved summary
accelsim-lite report latest
```

---

## Bundled workloads

Four workloads ship with the repo to cover the primary performance scenarios:

| File                           | Character                                                    |
|--------------------------------|--------------------------------------------------------------|
| `workloads/compute_heavy.csv`  | High compute-unit contention, minimal memory traffic         |
| `workloads/memory_heavy.csv`   | High memory-port contention, minimal compute                 |
| `workloads/queue_pressure.csv` | Queue-capacity-sensitive workload for observing pressure and backpressure |
| `workloads/mixed.csv`          | Balanced mix of COMPUTE, LOAD, and STORE with cross-deps     |

---

## Charting

`tools/plot_report.py` reads the exported CSVs and generates two charts:

- **Pipeline utilization** — busy cycles per stage
- **Stall breakdown** — stall counts by type

```sh
python3 tools/plot_report.py reports/latest/
```

Requires `matplotlib`.

---

## Docs

| File                  | Contents                                            |
|-----------------------|-----------------------------------------------------|
| `docs/model.md`       | Formal description of the simulation model          |
| `docs/limitations.md` | Explicit scope boundaries and known approximations  |

---

## Implementation status

### Fully implemented

- Core workload simulation and staged pipeline
- Dependency-aware instruction issuing
- Resource contention tracking (compute units, memory ports, queues)
- Stall accounting with five stall categories
- Metrics collection and `top_bottleneck` identification
- Report export (txt, json, csv)
- CLI: `run`, `compare`, `benchmark all`, `report latest`
- Bundled workloads (four classes)
- Python charting helper
- Simulation model and limitations docs

### Partially implemented / needs cleanup

- Full CMake integration with pre-existing repo targets
- Unified test target setup (older and newer targets conflict during `cmake --build build -j`)
- Clean end-to-end test build without legacy target conflicts

### Not yet implemented

- Richer workload comparison tables
- Before/after tuning report formatting
- Deterministic scheduling policy documentation
- Architecture and event-flow diagrams
- Regression baseline snapshots
- Polished benchmark artifacts

---

## Project structure

```
accelsim-lite/
├── workloads/
│   ├── compute_heavy.csv
│   ├── memory_heavy.csv
│   ├── queue_pressure.csv
│   └── mixed.csv
├── tools/
│   └── plot_report.py
├── docs/
│   ├── model.md
│   └── limitations.md
├── reports/
│   └── latest/
│       ├── summary.txt
│       ├── summary.json
│       ├── utilization.csv
│       ├── stalls.csv
│       ├── queue_occupancy.csv
│       └── completed_trace.csv
└── src/
    └── (C++ source)
```

---

## License

See `LICENSE` for details.

## Scope and modeling assumptions

AccelSim-Lite is **not a cycle-accurate GPU simulator** and does not model:
- warp scheduling
- cache-line behavior
- Tensor Core instruction timing
- coalescing effects
- occupancy at the SM / warp level
- kernel launch overheads
- vendor-specific compiler and runtime optimizations

What it **does** model faithfully at the workload level:
- bottleneck shifts between compute, memory, and dependency pressure
- queue buildup and backpressure propagation
- throughput and latency sensitivity to constrained resources
- how instruction dependencies reduce effective parallelism

This makes it useful for:
- early-stage accelerator performance reasoning
- capacity and bottleneck analysis
- comparing workload classes before lower-level kernel tuning

## Why this matters for AI inference

This matters directly for modern AI infrastructure: NVIDIA describes LLM inference as both memory- and compute-intensive, and notes that decode can be **memory-bandwidth-bound** because GPUs spend significant time moving KV-cache data rather than computing. In that sense, the simulator's memory-heavy mode is a useful abstraction for reasoning about transformer inference paths where bandwidth, dependency chains, and queue pressure dominate end-to-end latency.

## 🧠 How to explain this in an interview

This system models **workload-level execution behavior**, not hardware cycles.

Key idea:
- Instructions form a dependency graph
- Execution is constrained by compute units, memory ports, and queue capacities
- Performance emerges from resource contention and dependency structure

What it shows:
- Why memory-bound workloads slow down (bandwidth bottleneck)
- Why dependency chains limit parallelism
- How queue pressure leads to backpressure and reduced throughput

What it does NOT do:
- It does not simulate warp-level GPU execution
- It does not model cache-line or instruction-level timing
- It is not calibrated to exact GPU hardware latencies

Think of it as:
→ a **performance reasoning tool**, not a hardware emulator

## Hardware / AI-infra validation artifacts

AccelSim-Lite is intentionally scoped for niche systems and performance roles:
- accelerator performance modeling
- AI infrastructure
- runtime/compiler-adjacent SWE
- hardware-performance analysis

### Workload comparison reports

Generated reports compare throughput, latency, and bottleneck shifts across:
- `compute_heavy`
- `memory_heavy`
- `mixed`
- `queue_pressure`

Artifacts:

```bash
python3 scripts/analysis/workload_comparison.py
python3 scripts/analysis/classify_workloads.py
python3 scripts/analysis/plot_workloads.py
python3 scripts/analysis/memory_sweep_report.py
Generated outputs:

reports/analysis/workload_comparison.json
reports/analysis/workload_comparison.md
reports/analysis/workload_classification.json
reports/analysis/throughput_by_workload.png
reports/analysis/latency_by_workload.png
reports/analysis/memory_sweep.json
reports/analysis/memory_sweep.md
Validation highlights

Observed workload behavior:

throughput range: 0.1395 -> 0.3333 ops/cycle
latency range: 10.0 -> 23.8333 cycles
memory-heavy throughput drop vs compute-heavy: 58.15%
memory-heavy latency increase vs compute-heavy: 2.38x
What-if memory sweep

Scaling memory ports on memory_heavy:

memory ports: 1 -> 4
latency improvement: 39.16%
throughput improvement: 87.03%
bottleneck shift:
NoMemoryPort -> WaitingDependency

This demonstrates that removing a memory bottleneck exposes the next limiting factor: dependency stalls.

Scope

AccelSim-Lite is not a cycle-accurate GPU simulator.

It is a workload-level performance model designed for:

throughput analysis
bottleneck reasoning
memory-vs-compute pressure analysis
accelerator workload comparison

It intentionally does not model:

warp scheduling
Tensor Core timing
cache-line behavior
GPU occupancy details
AI inference relevance

Transformer inference often contains memory-bound phases, especially KV-cache-heavy decoding and attention execution.

AccelSim-Lite models the same high-level behavior:

throughput and latency degrade under memory pressure
increasing bandwidth shifts bottlenecks rather than eliminating them


### Dashboard-style report

Additional dashboard artifacts:

- `reports/analysis/dashboard.md`
- `reports/analysis/memory_sweep_latency.png`
- `reports/analysis/memory_sweep_throughput.png`
- `reports/analysis/memory_sweep_stalls.png`

These summarize workload classification, latency/throughput trends, memory-port scaling, and bottleneck transitions in one reviewer-friendly report.

## Claims boundary

AccelSim-Lite is a workload-level performance model, not a cycle-accurate GPU simulator.

Safe claims:
- workload-level throughput and latency modeling
- bottleneck classification
- memory-vs-dependency pressure analysis
- what-if resource sensitivity sweeps
- dashboard-style performance reporting

Do not claim:
- CUDA execution
- Tensor Core simulation
- warp scheduling
- hardware-accurate NVIDIA/A100/H100 latency prediction
- real MLPerf validation
- production LLM inference optimization

See `docs/claims_boundary.md` for the full claims boundary.

## Quality gate

Run the full local quality gate:

```bash
./scripts/quality/run_quality_gate.sh
he quality gate checks:

CMake build
validation workloads
workload comparison artifacts
memory sweep artifacts
dashboard report generation
claims boundary enforcement

This keeps the project reproducible and prevents inflated hardware/GPU claims.

Static workload visualizer

AccelSim-Lite includes a static HTML workload visualizer:

python3 scripts/visualization/build_workload_visualizer.py
open reports/visualization/workload_visualizer.html

The visualizer summarizes:

pipeline stages
workload throughput and latency
memory-bound vs dependency-bound classifications
memory-port what-if sweep
bottleneck transitions
generated latency/throughput charts


## Timing validation artifacts

AccelSim-Lite includes simulator-cycle timing validation artifacts:

- `validation/timing_regression_report.json`
- `validation/clock_cycle_breakdown.md`

These reports capture:
- cycle drift against deterministic simulator baselines
- pipeline-stage busy-cycle timing
- stall timing by bottleneck type

Scope note: these are simulator-cycle validation artifacts, not real hardware timing claims.

## Communication-pressure analysis

AccelSim-Lite includes synthetic workload-level communication-pressure experiments:

- `analysis/collective_bandwidth_sweep.json`
- `analysis/communication_bottleneck_report.md`

The experiment compares:
- compute-heavy execution
- transfer-heavy all-gather-style pressure
- reduction-style transfer pressure
- queue-delay growth under simulated communication pressure
- bottleneck transitions across compute-limited, memory-port-limited, and communication-pressure-limited regimes

Scope note: this is a synthetic workload-level model and does not claim real NCCL, EFA, PCIe, NVLink, or hardware collective benchmarking.

## Threaded workload runner

AccelSim-Lite includes a standalone C++ threaded workload experiment:

- `parallel_execution/threaded_workload_runner.cpp`
- `parallel_execution/threaded_worker_sweep.json`
- `parallel_execution/threaded_workload_report.md`

The experiment uses `std::thread` and `std::mutex` to model:
- 1 / 2 / 4 worker-thread execution
- balanced vs imbalanced synthetic workload partitions
- shared queue coordination
- worker-level simulated-cycle accounting
- bottleneck classifications such as serial-bound, worker-imbalance-limited, contention-limited, and parallelism-improved

Scope note: this experiment demonstrates C++ threading and workload-level scaling behavior. It is not an OS scheduler or hardware timing benchmark.

## Digital verification lab

AccelSim-Lite includes a small SystemVerilog FIFO verification lab:

- `digital_verification_lab/rtl/simple_fifo.sv`
- `digital_verification_lab/tb/fifo_tb.sv`
- `digital_verification_lab/tb/fifo_assertions.sv`
- `digital_verification_lab/docs/fifo_verification_plan.md`
- `digital_verification_lab/docs/fifo_coverage_notes.md`
- `digital_verification_lab/docs/fifo_verification_report.md`

The lab demonstrates:
- FIFO RTL familiarity
- directed SystemVerilog testbench construction
- assertion-style checks
- reset, full, empty, overfill, under-read, and simultaneous read/write scenarios
- scenario coverage planning

Scope note: this is a directed SystemVerilog verification lab, not a UVM environment, formal proof, synthesis flow, or timing-closure signoff.

## Execution engine lab

AccelSim-Lite includes a compact C++ stack-bytecode interpreter lab:

- `execution_engine_lab/stack_bytecode_interpreter.cpp`
- `execution_engine_lab/bytecode_workloads.json`
- `execution_engine_lab/interpreter_dispatch_report.md`
- `execution_engine_lab/opcode_frequency_report.json`

The lab models a small stack-bytecode instruction set:
- `PUSH_CONST`
- `LOAD`
- `ADD`
- `SUB`
- `MUL`
- `JUMP_IF_ZERO`
- `RETURN`

It reports:
- instruction count
- opcode frequency
- dispatch-step count
- branch-heavy vs arithmetic-heavy traces
- basic-block length
- stack-depth behavior

Scope note: this is a toy interpreter-style execution lab. It does not claim compiler, JIT, V8, or WebAssembly runtime implementation experience.

## Bytecode optimization lab

AccelSim-Lite includes a toy bytecode optimization pass:

- `execution_engine_lab/bytecode_optimizer.cpp`
- `execution_engine_lab/optimized_trace_comparison.json`
- `execution_engine_lab/bytecode_optimization_report.md`

The pass performs constant folding for simple stack-bytecode patterns such as:

```text
PUSH_CONST 2
PUSH_CONST 3
ADD
into:

PUSH_CONST 5

It compares before/after:

instruction count
opcode distribution
branch/control-flow behavior
stack-depth proxy
dispatch-step reduction

Scope note: this is a toy bytecode transformation lab. It does not claim compiler, JIT, V8, or WebAssembly runtime implementation experience.

## Tiered execution and deoptimization lab

AccelSim-Lite includes a toy tiered-execution and deoptimization simulation:

- `execution_engine_lab/tiered_execution_sim.cpp`
- `execution_engine_lab/hot_trace_profiles.json`
- `execution_engine_lab/speculative_guard_cases.json`
- `execution_engine_lab/tiering_and_deopt_report.md`

The lab models:
- baseline execution
- hot-trace detection
- repeated branch-path tracking
- basic-block reuse
- speculative optimized-path execution
- guard checks
- fallback to baseline execution
- deoptimization event reporting

Scope note: this is a toy tiered-execution and deoptimization simulation. It does not claim JIT, V8, WebAssembly runtime, or production compiler implementation experience.
