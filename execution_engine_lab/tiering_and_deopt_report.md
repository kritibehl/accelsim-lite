# Tiering and Deoptimization Report

> Scope: toy tiered-execution and deoptimization simulation for stack-bytecode traces. This is not a JIT, V8 engine, WebAssembly runtime, or production compiler implementation.

## Hot trace profiles

| Trace | Invocations | Opcode Count | Repeated Branch Path | Basic Block Reuse | Dominant Branch | Hot Trace |
|---|---:|---:|---:|---:|---|---|
| arith_trace | 5 | 40 | 5 | 5 | none | true |
| branch_trace | 5 | 31 | 4 | 4 | taken | true |
| mixed_trace | 4 | 40 | 4 | 4 | not_taken | true |

## Tiering summary

| Trace | Optimized Attempts | Optimized Hits | Deopts | Hit Rate | Final Status |
|---|---:|---:|---:|---:|---|
| arith_trace | 2 | 1 | 1 | 50.00% | hot-with-deopt-events |
| branch_trace | 2 | 1 | 1 | 50.00% | hot-with-deopt-events |
| mixed_trace | 1 | 1 | 0 | 100.00% | hot-stable-optimized |

## Guard failures and fallback events

| Trace | Invocation | Tier | Optimized Attempted | Optimized Taken | Guard Failed | Reason | Fallback |
|---|---:|---|---|---|---|---|---|
| arith_trace | 4 | optimized_candidate | true | true | false | none | none |
| arith_trace | 5 | deoptimized-to-baseline | true | false | true | operand-type-changed | fallback_to_baseline_execution |
| branch_trace | 4 | deoptimized-to-baseline | true | false | true | branch-direction-changed | fallback_to_baseline_execution |
| branch_trace | 5 | optimized_candidate | true | true | false | none | none |
| mixed_trace | 4 | optimized_candidate | true | true | false | none | none |

## Interpretation

- Hot traces are detected from repeated invocation counts, repeated branch paths, and basic-block reuse.
- Optimized-path execution is attempted only after a trace exceeds the hot threshold.
- Guard checks model speculative assumptions such as stable numeric operands, stable branch direction, and stable basic-block shape.
- Guard failures trigger fallback to baseline execution and are recorded as deoptimization events.
- This demonstrates runtime feedback and optimization tradeoff reasoning without claiming production JIT/compiler implementation experience.
