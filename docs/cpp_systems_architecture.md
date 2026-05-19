# AccelSim-Lite C++ Systems Architecture

```mermaid
flowchart TD
    A[CSV workload input] --> B[C++ CLI runner]
    B --> C[Pipeline simulator]
    C --> D[Compute / memory / queue model]
    D --> E[Latency + throughput reports]
    D --> F[Stall distribution reports]
    D --> G[Queue occupancy traces]

    H[Execution engine labs] --> I[Bytecode interpreter]
    H --> J[Optimizer + semantic validation]
    H --> K[Tiering / deopt simulation]
    H --> L[Wasm-style validation]
    H --> M[Runtime regression gate]

    N[Python automation] --> O[Benchmark reports]
    N --> P[Charts + dashboards]
    N --> Q[ML systems studies]

    R[GoogleTest] --> S[Boundary + invalid config tests]
    R --> T[Queue pressure tests]
    R --> U[Determinism tests]

    V[Quality gate] --> W[CMake build]
    V --> X[Validation workloads]
    V --> Y[Runtime regression gate]
    V --> Z[Claims check]
