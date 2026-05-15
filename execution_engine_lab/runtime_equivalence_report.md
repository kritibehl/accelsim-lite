# Runtime Equivalence Report

> Scope: toy differential runtime validation for baseline, optimized, and deoptimized stack-bytecode execution paths. This is not a production compiler, JIT, VM, V8, or WebAssembly validation harness.

| Trace | Baseline Result/Error | Optimized Result/Error | Deopt Result/Error | Match? | Notes |
|---|---|---|---|---|---|
| arithmetic_constant_fold_equivalence | 5 | 5 | 5 | PASS | constant folding preserved return value |
| branch_taken_equivalence | 1 | 1 | 1 | PASS | taken branch result preserved |
| branch_not_taken_equivalence | 9 | 9 | 9 | PASS | not-taken branch result preserved |
| hot_trace_optimized_equivalence | 27 | 27 | 27 | PASS | hot optimized arithmetic path preserved result |
| guard_failure_deopt_equivalence | 7 | 999 | 7 | PASS | optimized path would be unsafe, deopt fallback preserves baseline output |
| invalid_trace_rejected_consistently | error:stack_underflow | error:stack_underflow | error:stack_underflow | PASS | malformed trace rejected consistently |

## What is compared

- returned value
- final stack contents
- error category
- branch outcome count
- stack depth at exit
- deopt fallback output correctness

## Safe interpretation

This lab demonstrates semantic-drift detection across baseline, optimized, and deoptimized toy runtime paths. It intentionally does not claim production compiler, VM, JIT, V8, or WebAssembly validation experience.
