# Performance Dashboard Walkthrough

## Purpose

The performance dashboard summarizes the major systems/performance experiments inside AccelSim-Lite and explains the engineering meaning behind the reported metrics.

Dashboard:

- `reports/performance_dashboard.html`

## Memory-port scaling

Observed:

- 39.16% latency improvement
- 87.03% throughput improvement
- bottleneck shift:
  - `NoMemoryPort`
  - →
  - `WaitingDependency`

### What this means

The workload initially spent most of its time stalled on memory-port pressure.

After scaling memory ports:
- memory contention reduced,
- throughput improved,
- and the dominant bottleneck shifted toward dependency delay.

This demonstrates:
- bottleneck-transition analysis,
- workload sensitivity modeling,
- and resource-scaling tradeoff reasoning.

## Runtime regression gate

Observed:
- semantic mismatches: 0
- malformed-trace false accepts: 0
- regression gate status: PASS

### What this means

The runtime validation infrastructure checks that:
- optimized execution preserves semantics,
- malformed execution traces are rejected consistently,
- and runtime behavior does not drift unexpectedly across experiments.

This models:
- execution-engine correctness validation,
- differential runtime testing,
- and regression-detection workflows.

## Cache/locality study

Observed:
- pointer-heavy traversal significantly slower than contiguous traversal
- AoS vs SoA layout tradeoffs measurable

### What this means

The experiment demonstrates how:
- indirection,
- layout decisions,
- and access patterns

can materially affect traversal efficiency and simulated locality behavior.

This is relevant to:
- systems programming,
- runtime systems,
- performance engineering,
- and hardware-adjacent software reasoning.

## KV-cache / token batching simulation

Observed:
- larger contexts increase simulated KV pressure
- memory-port scaling reduces KV bottlenecks
- decode dependency pressure remains important after scaling

### What this means

The study models how transformer-style serving workloads can shift between:
- memory pressure,
- dependency pressure,
- and batching overhead.

This demonstrates:
- inference-systems reasoning,
- bottleneck classification,
- and workload-behavior experimentation.

## Claims boundary

AccelSim-Lite is:
- a systems/runtime experimentation platform,
- a simulator-driven workload study environment,
- and a runtime/performance validation project.

It is not:
- a real GPU benchmark,
- a production inference serving stack,
- a V8 implementation,
- or a production compiler/runtime system.
