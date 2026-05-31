# Optimization Explorer

> Scope: readable baseline-vs-optimized comparison over AccelSim-Lite benchmark studies.

| Experiment | Optimization | Latency change | Speedup | Throughput change | Decision |
|---|---|---:|---:|---:|---|
| metadata_lookup_path | restore faster metadata lookup path | -33.87% | 1.512x | 14.29% | improved |
| cache_locality_layout | move pointer-heavy traversal toward mixed-locality layout | -72.53% | 3.64x | 264.0% | improved |
| threading_queue_partitioning | reduce shared queue contention | -18.18% | 1.222x | 8.54% | improved |

## What this proves

This explorer turns performance experiments into a clear before/after story: baseline, optimized, speedup, throughput movement, and regression/improvement decision.

## Claims boundary

This is an educational optimization-comparison layer over simulator-derived reports. It does not claim production autotuning or hardware-level optimization.
