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