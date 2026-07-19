# Compiler Optimization Comparison

> Results are machine-specific deterministic benchmark observations.

| Optimization | Flag | Runtime ms | Throughput items/s | p50 µs | p95 µs | p99 µs |
|---|---|---:|---:|---:|---:|---:|
| O0 | `-O0` | 0.8416 | 118823551.78 | 374.46 | 672.44 | 700.45 |
| O2 | `-O2` | 0.6894 | 145050093.05 | 50.08 | 181.52 | 189.29 |
| O3 | `-O3` | 0.3643 | 274505067.36 | 23.62 | 52.50 | 57.44 |

## Interpretation

- `-O0` provides an unoptimized development baseline
- `-O2` enables broadly useful compiler optimizations
- `-O3` enables more aggressive optimization
- results should be compared on the same machine
- repeated runs are needed before making strong performance claims
