# accelsim-lite

## Execution Engine & Runtime Experiments

This lab explores simplified runtime-system concepts relevant to interpreter-style execution and dynamic-language optimization:

1. Stack-bytecode interpreter and opcode dispatch profiling
2. Bytecode optimization via constant folding and dispatch-step reduction
3. Feedback-driven tiered execution with guard failures and deoptimization fallbacks
4. JavaScript-like object-shape and inline-cache specialization behavior
5. WebAssembly-style stack validation for structured control-flow traces

These are educational runtime experiments, not a JIT compiler, not V8, and not a WebAssembly runtime.

| Lab | What it proves | Key artifact |
|---|---|---|
| Bytecode interpreter | dispatch/runtime profiling | `execution_engine_lab/interpreter_dispatch_report.md` |
| Bytecode optimizer | instruction-stream transformation | `execution_engine_lab/bytecode_optimization_report.md` |
| Tiering/deopt | runtime feedback + fallback behavior | `execution_engine_lab/tiering_and_deopt_report.md` |
| Object shapes / inline cache | JS-like runtime specialization | `execution_engine_lab/inline_cache_transition_report.md` |
| Wasm validator | stack/control-flow correctness | `execution_engine_lab/wasm_validation_report.md` |

See also:
- `execution_engine_lab/runtime_experiment_summary.md`
- `docs/runtime_execution_engine_overview.md`


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

## Object shape and inline-cache lab

AccelSim-Lite includes a toy JavaScript-like object-shape and inline-cache simulation:

- `execution_engine_lab/object_shape_cache_sim.cpp`
- `execution_engine_lab/property_access_profiles.json`
- `execution_engine_lab/inline_cache_transition_report.md`

The lab models:
- simplified object shapes
- property layouts and repeated access sites
- monomorphic, polymorphic, and megamorphic-style cache states
- shape invalidations
- generic lookup fallback behavior

Scope note: this is a toy runtime simulation. It does not claim V8 implementation, JavaScript engine internals, or production inline-cache work.

## WebAssembly-style stack validation lab

AccelSim-Lite includes a toy WebAssembly-style stack validation lab:

- `execution_engine_lab/wasm_style_stack_validator.cpp`
- `execution_engine_lab/wasm_control_flow_cases.json`
- `execution_engine_lab/wasm_validation_report.md`

The lab validates structured control-flow traces using a small Wasm-style instruction subset:
- `i32.const`
- `i32.add`
- `local.get`
- `block`
- `loop`
- `br_if`
- `return`

It reports:
- valid vs invalid traces
- operand-stack height transitions
- control-flow depth transitions
- branch-target validation outcomes
- malformed trace classifications such as stack underflow, type mismatch, invalid branch depth, and unterminated block

Scope note: this is a toy WebAssembly-style validation lab. It does not claim real WebAssembly runtime, compiler, validator, V8, or JIT implementation experience.

## Runtime equivalence validation

AccelSim-Lite includes a toy runtime equivalence harness:

- `execution_engine_lab/runtime_equivalence_harness.cpp`
- `execution_engine_lab/equivalence_trace_cases.json`
- `execution_engine_lab/runtime_equivalence_report.md`
- `execution_engine_lab/runtime_equivalence_summary.json`

The harness compares baseline, optimized, tiered-style, and deoptimized stack-bytecode execution paths across:
- returned values
- final stack contents
- error categories
- branch outcomes
- stack depth at exit
- fallback correctness after unsafe optimized paths

Scope note: this is a toy differential runtime-validation harness. It does not claim production compiler, VM, JIT, V8, or WebAssembly validation experience.

## WebAssembly-style malformed-trace regression pack

AccelSim-Lite includes a toy malformed-trace regression workflow for WebAssembly-style validation:

- `execution_engine_lab/wasm_malformed_trace_regression_cases.json`
- `execution_engine_lab/run_wasm_validation_regression.py`
- `execution_engine_lab/wasm_regression_report.md`
- `execution_engine_lab/wasm_regression_summary.json`

The regression pack checks deterministic rejection behavior for:
- stack underflow
- type mismatch
- invalid branch depth
- unterminated block
- unterminated loop
- unexpected block end
- malformed return shape

Scope note: this is a toy WebAssembly-style validation-regression workflow. It does not claim production WebAssembly runtime, compiler, validator, V8, or JIT implementation experience.

## CI-style execution-engine regression gate

AccelSim-Lite includes a CI-style runtime regression gate:

- `execution_engine_lab/runtime_regression_baseline.json`
- `execution_engine_lab/run_runtime_regression_gate.py`
- `execution_engine_lab/runtime_regression_report.md`
- `execution_engine_lab/runtime_regression_summary.json`

The gate compares current runtime experiment outputs against checked-in expectations for:
- semantic-equivalence drift
- runtime-equivalence mismatches
- malformed Wasm-style false accepts
- deoptimization guard-failure classification drift
- inline-cache terminal-state regressions

Scope note: this is a toy execution-engine regression gate. It does not claim production compiler, VM, JIT, V8, or WebAssembly validation infrastructure.

## Runtime metadata lookup benchmark

AccelSim-Lite includes a runtime metadata lookup benchmark:

- `execution_engine_lab/dispatch_lookup_structure_benchmark.cpp`
- `execution_engine_lab/lookup_structure_benchmark_report.md`
- `execution_engine_lab/lookup_structure_benchmark_summary.json`

The benchmark compares:
- hash-table-backed metadata lookup via `std::unordered_map`
- tree-backed metadata lookup via `std::map`

Across:
- opcode dispatch metadata
- object-shape metadata
- inline-cache-style metadata workloads

Scope note: this is an educational runtime metadata experiment and not a V8 benchmark or production engine benchmark.

## Node/V8 runtime observation notes

AccelSim-Lite includes a real Node/V8 optimization-trace observation that maps public V8 runtime behavior to the toy runtime labs:

- `docs/real_v8_observations/node_v8_optimization_trace.md`
- `docs/real_v8_observations/artifacts/stable_property_access.js`
- `docs/real_v8_observations/artifacts/stable_property_access_trace.txt`

The observation records a repeated property-access function being marked hot/stable and optimized to MAGLEV under Node/V8 tracing flags.

Scope note: this is a Node/V8 observation artifact only. It does not claim V8 source build, d8 usage, V8 internals modification, Chromium contribution, or production JavaScript engine work.

## Node/V8 shape-instability observation

AccelSim-Lite includes a second Node/V8 observation artifact for property-shape instability:

- `docs/real_v8_observations/node_v8_shape_instability_trace.md`
- `docs/real_v8_observations/artifacts/shape_instability_deopt.js`
- `docs/real_v8_observations/artifacts/shape_instability_deopt_trace.txt`

The experiment warms up stable property access, then introduces a different object shape to connect real Node/V8 trace behavior with AccelSim-Lite's toy object-shape and inline-cache experiments.

Scope note: this is a Node/V8 observation artifact only. It does not claim d8 usage, V8 source build, V8 internals modification, Chromium contribution, or production JavaScript engine work.

## ML systems study: simulated LLM inference bottlenecks

AccelSim-Lite includes a simulated ML systems study of transformer-style inference workload behavior:

- `ml_systems_studies/llm_inference_workload_profiles.json`
- `ml_systems_studies/run_prefill_decode_bottleneck_sweep.py`
- `ml_systems_studies/llm_inference_bottleneck_results.json`
- `ml_systems_studies/llm_inference_bottleneck_report.md`
- `ml_systems_studies/research_note_prefill_decode_pressure.md`

The study compares simulated:
- prefill-heavy workloads
- decode-heavy workloads
- mixed serving traces
- queue-pressure serving traces

It sweeps:
- memory ports
- queue depth
- dependency delay

and reports latency, throughput, queue delay, stall distributions, and bottleneck transitions.

Scope note: this is a simulator-based ML systems study. It does not claim real LLM serving, real accelerator benchmarking, or production inference measurements.

## C++ systems/performance flagship artifacts

AccelSim-Lite includes packaging artifacts for C++ systems, runtime, and performance roles:

- `docs/cpp_systems_architecture.md`
- `docs/limitations.md`
- `docs/cpu_cache_locality_note.md`
- `scripts/dev/run_flagship_demo.sh`
- `scripts/dev/generate_gtest_report.sh`
- `reports/flagship/gtest_report.md`
- `reports/flagship/performance_dashboard.md`

These artifacts summarize the project architecture, claims boundary, GoogleTest/CTest workflow, performance-dashboard proof points, runtime regression gates, threading/queue experiments, ML systems studies, and metadata lookup benchmarks.

## Extended C++ systems/performance studies

AccelSim-Lite includes additional systems/performance artifacts:

- `docs/performance_systems/sanitizers_and_profiling.md`
- `docs/performance_systems/threading_and_contention_notes.md`
- `performance_systems/cache_locality_experiment.py`
- `performance_systems/cache_locality_report.md`
- `performance_systems/cache_locality_results.json`
- `performance_systems/kv_cache_batching_sim.py`
- `performance_systems/kv_cache_batching_report.md`
- `performance_systems/kv_cache_batching_results.json`

These cover sanitizer-aware build workflows, profiling notes, cache/locality-adjacent access-pattern experiments, threading/queue-contention notes, and simulated KV-cache/token-batching inference pressure.

Scope note: these are educational systems/performance studies and do not claim production profiling, hardware-cache measurement, or real LLM serving benchmarks.

## Performance dashboard

AccelSim-Lite includes an HTML benchmark landing page:

- `reports/performance_dashboard.html`

The dashboard summarizes:
- memory-port latency/throughput improvements
- runtime regression gate status
- malformed-trace false accepts
- cache/locality access-pattern results
- KV-cache/token-batching pressure simulations

Scope note: dashboard metrics are simulator-derived or educational workflow results, not hardware benchmark measurements.

## Dashboard walkthrough and screenshot guidance

Additional dashboard explanation artifacts:

- `docs/performance_systems/performance_dashboard_walkthrough.md`
- `reports/assets/dashboard_screenshot_instructions.md`

These explain:
- bottleneck-transition meaning
- runtime regression validation
- cache/locality tradeoffs
- KV-cache/token-batching pressure behavior
- and recommended dashboard screenshot capture workflow.

## What this proves

AccelSim-Lite includes a concise proof page and concurrency microbenchmark:

- `docs/performance_systems/what_this_proves.md`
- `performance_systems/threading_microbenchmark.py`
- `performance_systems/threading_microbenchmark_report.md`
- `performance_systems/threading_microbenchmark_results.json`
- `reports/assets/performance_dashboard_screenshot_note.md`

These artifacts summarize the C++ systems, runtime, performance, ML-systems, concurrency, and validation signals demonstrated by the project.

## Makefile and CI workflow

AccelSim-Lite includes a Makefile and GitHub Actions workflow for reproducible C++ systems/performance validation.

Common commands:

```bash
make build
make test
make benchmark
make dashboard
make ml-study
make quality
CI workflow:

.github/workflows/cpp-quality.yml

The workflow builds the C++ targets, runs tests, executes benchmark/report generators, and regenerates the performance dashboard.

## Jenkins-style CI and CMake build matrix

AccelSim-Lite includes Jenkins-style and CMake build-profile automation:

- `Jenkinsfile`
- `ci/quality_gate.sh`
- `ci/quality_gate_report.md`
- `cmake/BuildProfiles.cmake`
- `ci/run_build_matrix.sh`
- `ci/build_matrix_report.md`

The quality gate covers:
- CMake configure/build
- GoogleTest/CTest validation
- benchmark execution
- runtime regression threshold checks
- report archival guidance
- failure diagnostics

The build matrix covers:
- Debug
- Release
- ASan
- UBSan
- warnings-as-errors

Scope note: these are local/CI-style build reliability workflows for C++ runtime/performance experiments.

## OS concepts validation demos

AccelSim-Lite includes small C++ OS-concepts demos for systems/performance roles:

- `os_concepts/threading_race_sim.cpp`
- `os_concepts/memory_pressure_sim.cpp`
- `os_concepts/run_os_concepts_demo.sh`
- `os_concepts/scheduler_latency_report.md`
- `tests/test_os_concepts.cpp`

These demos cover thread contention, mutex/atomic correctness, memory-pressure-style working-set behavior, scheduler-style latency reporting, and GoogleTest-backed regression checks.

Scope note: these are educational OS-concepts demos and do not claim kernel development, production OS scheduler implementation, or hardware memory profiling.

## Cache hierarchy behavior study

AccelSim-Lite includes a small simulated cache hierarchy study:

- `cache_behavior/run_cache_hierarchy_study.py`
- `cache_behavior/cache_hierarchy_results.json`
- `cache_behavior/cache_hierarchy_report.md`
- `cache_behavior/cache_latency_chart.svg`

The study models:
- L1 hit behavior
- L2 hit behavior
- memory access latency
- locality profiles such as hot working sets, mixed locality, streaming access, and pointer chasing

Scope note: this is a simulated cache hierarchy study and does not claim hardware-counter measurement or CPU microarchitecture benchmarking.

## Simulated NUMA locality study

AccelSim-Lite includes a small simulated NUMA local-vs-remote memory study:

- `numa_behavior/run_numa_latency_study.py`
- `numa_behavior/numa_latency_results.json`
- `numa_behavior/numa_latency_report.md`

The study models local-memory access, balanced cross-socket-style access, and remote-heavy access to reason about latency deltas and memory-placement tradeoffs.

Scope note: this is a simulated NUMA-locality study and does not claim real NUMA hardware measurement or CPU performance-counter profiling.
