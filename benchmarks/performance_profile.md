# C++ Performance Profile

## Scope

This report documents AccelSim-Lite performance benchmarking methodology for deterministic pipeline simulation runs.

It is intended as a C++/systems performance artifact, not a hardware benchmark.

## Benchmark methodology

The benchmark compares deterministic workload execution under repeatable simulator settings.

Measured signals:

- runtime comparison
- throughput
- latency
- CPU time
- compiler-flag behavior
- workload execution consistency

## Workload classes

| Workload | Purpose |
|---|---|
| compute_heavy | measures instruction execution and dependency behavior |
| memory_heavy | stresses memory-port and stall behavior |
| mixed_workload | combines compute and memory pressure |
| queue_pressure | models scheduler and queue contention |
| metadata_lookup | measures runtime lookup-path sensitivity |

## Compiler profiles

| Profile | Flags | Purpose |
|---|---|---|
| Debug | `-O0 -g` | correctness and debug visibility |
| Release | `-O3 -DNDEBUG` | optimized runtime comparison |
| ASan | `-fsanitize=address` | memory-safety validation |
| UBSan | `-fsanitize=undefined` | undefined-behavior validation |
| Warnings-as-errors | `-Wall -Wextra -Werror` | build hygiene |

## Representative benchmark signals

| Signal | Example value | Meaning |
|---|---:|---|
| metadata lookup baseline p95 | 410 ns | baseline runtime lookup latency |
| metadata lookup candidate p95 | 620 ns | candidate runtime lookup latency |
| p95 regression | +51.22% | release-blocking latency drift |
| baseline throughput | 2,400,000 ops/sec | reference throughput |
| candidate throughput | 2,100,000 ops/sec | candidate throughput |
| throughput change | -12.5% | regression threshold exceeded |
| cache locality speedup opportunity | 3.64x | potential gain from improved locality |
| CAD flow lint failures | 0 | validation automation signal |

## Runtime comparison

| Comparison | Result |
|---|---|
| baseline vs candidate metadata lookup | candidate regressed |
| pointer-heavy vs mixed-locality access | pointer-heavy slower |
| local vs remote-style memory access | remote-heavy access slower |
| single-worker vs multi-worker queue model | scaling limited by contention |

## CPU time interpretation

CPU time is interpreted as a methodology signal for deterministic simulator workloads. The project focuses on relative comparisons and regression detection rather than claiming machine-independent absolute performance.

## Latency / throughput interpretation

Latency and throughput are reported as benchmark-study outputs. The primary engineering goal is to detect drift and explain bottlenecks:

- p95 latency increase → possible performance regression
- throughput drop → release-risk signal
- cache-locality slowdown → memory-layout concern
- queue scaling flattening → contention concern

## Validation workflow

Recommended local workflow:

```bash
make build
make test
make benchmark
make dashboard
Recommended CI workflow:

.github/workflows/cpp-quality.yml
.github/workflows/cad_flow_ci.yml
Claims boundary

This report documents C++ performance benchmarking methodology and representative simulator-derived signals. It does not claim hardware-counter profiling, production benchmarking, Graviton-specific tuning, or accelerator performance measurement.
