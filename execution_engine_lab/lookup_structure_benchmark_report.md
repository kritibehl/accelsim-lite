# Runtime Lookup Structure Benchmark

> Scope: educational data-structure tradeoff experiment for runtime metadata lookup. Not a V8 benchmark or production engine benchmark.

| Workload | Structure | Entries | Lookups | Hits | Misses | Elapsed ms |
|---|---|---|---|---|---|---|
| opcode_dispatch_small | unordered_map_hash_table | 16 | 100000 | 95085 | 4915 | 17.5967 |
| opcode_dispatch_small | map_tree_index | 16 | 100000 | 95085 | 4915 | 47.7590 |
| opcode_dispatch_medium | unordered_map_hash_table | 128 | 200000 | 179890 | 20110 | 34.8213 |
| opcode_dispatch_medium | map_tree_index | 128 | 200000 | 179890 | 20110 | 143.4610 |
| shape_metadata_diverse | unordered_map_hash_table | 512 | 300000 | 240373 | 59627 | 57.7286 |
| shape_metadata_diverse | map_tree_index | 512 | 300000 | 240373 | 59627 | 261.4867 |
| inline_cache_state_diverse | unordered_map_hash_table | 1024 | 400000 | 279989 | 120011 | 74.8703 |
| inline_cache_state_diverse | map_tree_index | 1024 | 400000 | 279989 | 120011 | 407.1053 |

## Tradeoff notes

- unordered_map models average-case hash-table lookup behavior
- map models ordered tree-backed lookup behavior
- shape-diverse workloads increase metadata lookup pressure
- runtime metadata access patterns can affect lookup efficiency
