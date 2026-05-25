# What AccelSim-Lite Proves

## C++ systems engineering

AccelSim-Lite demonstrates C++17 systems implementation with CMake builds, GoogleTest coverage, CLI workflows, runtime experiments, and reproducible validation reports.

## Performance engineering

The project measures latency, throughput, stall causes, queue pressure, bottleneck shifts, memory-port sensitivity, and metadata lookup behavior across simulated workloads.

## Runtime engineering

The execution-engine labs cover stack-bytecode interpretation, opcode dispatch profiling, bytecode optimization, tiered execution, deoptimization fallback, object-shape/inline-cache modeling, and runtime regression gates.

## ML systems / inference systems

The simulator includes transformer-style inference workload studies for prefill-heavy, decode-heavy, mixed-serving, queue-pressure, KV-cache, and token-batching behavior.

## Concurrency / scheduling

The project includes threaded workload execution, worker-scaling reports, queue-pressure behavior, scheduler comparisons, and contention-oriented experiments.

## Validation discipline

AccelSim-Lite includes GoogleTest/CTest reports, semantic-equivalence validation, malformed-trace regression checks, timing-regression reports, and CI-style quality gates.

## What it does not prove

It does not claim:
- production GPU simulation
- real accelerator benchmarking
- production LLM serving
- V8 implementation
- WebAssembly runtime implementation
- production compiler/JIT engineering
- hardware-counter CPU cache measurement
