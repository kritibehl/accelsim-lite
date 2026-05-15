# Runtime Execution Engine Overview

```mermaid
flowchart TD
    A[Stack bytecode trace] --> B[Interpreter dispatch]
    B --> C[Opcode profiling]
    C --> D[Bytecode optimizer]
    D --> E[Runtime feedback / hot trace detection]
    E --> F[Speculative optimized path]
    F --> G{Guard check}
    G -->|pass| H[Optimized execution]
    G -->|fail| I[Deopt fallback to baseline]

    J[Dynamic property access] --> K[Object shape tracking]
    K --> L[Inline cache specialization]
    L --> M{Shape diversity}
    M -->|one shape| N[Monomorphic fast path]
    M -->|few shapes| O[Polymorphic cache]
    M -->|many shapes| P[Generic fallback]

    Q[Wasm-style structured trace] --> R[Stack/control-flow validation]
    R --> S{Valid?}
    S -->|yes| T[Accept toy trace]
    S -->|no| U[Reject malformed trace]
