# Runtime Experiment Summary

> Scope: educational runtime/execution-engine experiments. These are not a JIT compiler, V8 implementation, JavaScript engine, or WebAssembly runtime.

| Experiment | Verified signal | Key artifact |
|---|---|---|
| Bytecode interpreter | Opcode dispatch profiling across arithmetic, branch-heavy, and mixed traces | `interpreter_dispatch_report.md` |
| Bytecode optimizer | 33.33% dispatch-step reduction on constant-folding traces | `bytecode_optimization_report.md` |
| Tiering/deopt | Hot-path promotion with guard-failure deopt fallback | `tiering_and_deopt_report.md` |
| Object shapes / inline cache | Megamorphic generic fallback after 5 unique shapes at one access site | `inline_cache_transition_report.md` |
| Wasm-style validator | Rejects stack underflow, type mismatch, invalid branch depth, and unterminated blocks | `wasm_validation_report.md` |

## Interview defense

These labs show a coherent runtime-systems learning arc:

1. execute stack bytecode
2. profile dispatch behavior
3. transform instruction streams
4. promote hot traces
5. guard optimized paths
6. fall back on failed assumptions
7. specialize dynamic property access
8. validate stack/control-flow correctness before execution

The goal is to demonstrate runtime reasoning and correctness awareness, not production compiler or VM implementation experience.
