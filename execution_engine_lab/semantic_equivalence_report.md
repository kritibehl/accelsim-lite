# Runtime Semantic Equivalence Report

> Scope: toy differential validation for baseline vs optimized stack-bytecode traces. This is not a production compiler validation harness.

| Case | Equivalent | Mismatch | Baseline Result | Optimized Result | Dispatch Delta | Branches B/O | Stack Depth B/O |
|---|---|---|---:|---:|---:|---:|---:|
| constant_fold_preserves_result | true | none | 20 | 20 | 2 | 0/0 | 2/2 |
| load_constant_fold_preserves_result | true | none | 27 | 27 | 2 | 0/0 | 3/2 |
| branch_shape_preserved | true | none | 42 | 42 | 2 | 1/1 | 2/1 |
| intentional_mismatch_detected | false | result_mismatch | 5 | 6 | 2 | 0/0 | 2/1 |

## Summary

- Equivalent optimized traces: 3
- Mismatched optimized traces detected: 1
- The harness catches result mismatches after optimization.
- Dispatch-step deltas show the runtime cost effect of optimized traces.

## Safe interpretation

This lab demonstrates differential validation thinking: optimized traces must preserve baseline semantics while reducing dispatch work. It is intentionally small and does not claim production compiler or VM validation experience.
