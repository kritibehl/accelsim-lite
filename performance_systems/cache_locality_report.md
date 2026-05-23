# Cache / Locality Experiment Report

> Scope: locality-adjacent experiment for access-pattern reasoning. This is not hardware cache measurement or CPU microarchitecture benchmarking.

| Experiment | Description | Items | Elapsed ms | Relative to contiguous |
|---|---|---:|---:|---:|
| contiguous_scan | contiguous list traversal | 250000 | 3.862 | 1.0x |
| strided_scan | strided access pattern | 250000 | 3.8297 | 0.9916x |
| pointer_heavy_scan | pointer-like dictionary chain traversal | 250000 | 99.0685 | 25.6521x |
| array_of_structs | array-of-structs-style object layout | 250000 | 49.0654 | 12.7047x |
| struct_of_arrays | structure-of-arrays-style layout | 250000 | 17.9809 | 4.6559x |

## Interpretation

- Contiguous scans model locality-friendly access.
- Strided and pointer-heavy scans model reduced locality and indirection.
- AoS vs SoA captures memory-layout tradeoff reasoning used in systems/performance work.

## Claims boundary

This experiment is Python-level and intended for systems reasoning. It does not claim hardware cache-counter measurement.
