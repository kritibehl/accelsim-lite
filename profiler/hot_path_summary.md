# Runtime Hot Path Summary

## Scope

Profiler-style summary for AccelSim-Lite runtime/performance experiments.

This is a simulated/profile-report artifact, not production sampled profiling.

## Hot paths

| Component | Share | Why it matters |
|---|---:|---|
| metadata_lookup | 31.4% | Runtime metadata lookup pressure across hash-table and tree-backed indexes |
| bytecode_dispatch | 24.8% | Interpreter-style dispatch overhead |
| cache_locality_scan | 18.6% | Memory layout and pointer-heavy access sensitivity |
| threading_queue_worker | 14.2% | Queue contention and worker synchronization behavior |
| wasm_validation | 11.0% | Structured trace validation and malformed-case checks |

## What this proves

This report makes runtime/performance bottlenecks visible across:
- dispatch overhead
- metadata lookup pressure
- cache-sensitive workloads
- threading contention
- validation costs

## Claims boundary

This is an educational profiler-style report. It does not claim production flamegraph profiling or hardware-counter analysis.
