# Interpreter Dispatch Report

> Scope: toy C++ stack-bytecode interpreter for execution-engine dispatch analysis. This is not a compiler, JIT, V8 engine, or WebAssembly runtime.

| Workload | Category | Result | Instructions | Dispatch Steps | Branches | Taken Branches | Avg Basic Block | Max Stack | Classification |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|
| arithmetic_heavy | arithmetic | 62 | 8 | 8 | 0 | 0 | 8.00 | 2 | arithmetic-heavy-dispatch |
| branch_zero_fast_path | branch_heavy | 42 | 4 | 4 | 1 | 1 | 2.00 | 1 | branch-heavy-dispatch |
| mixed_control_arithmetic | mixed | 20 | 10 | 10 | 1 | 0 | 5.00 | 2 | mixed-dispatch |

## Opcode mix

### arithmetic_heavy

| Opcode | Count |
|---|---:|
| ADD | 1 |
| LOAD | 3 |
| MUL | 2 |
| PUSH_CONST | 1 |
| RETURN | 1 |

### branch_zero_fast_path

| Opcode | Count |
|---|---:|
| JUMP_IF_ZERO | 1 |
| LOAD | 1 |
| PUSH_CONST | 1 |
| RETURN | 1 |

### mixed_control_arithmetic

| Opcode | Count |
|---|---:|
| ADD | 1 |
| JUMP_IF_ZERO | 1 |
| LOAD | 4 |
| MUL | 1 |
| PUSH_CONST | 1 |
| RETURN | 1 |
| SUB | 1 |

## Interpretation

- Arithmetic-heavy traces spend most dispatches on stack operations such as LOAD, ADD, MUL, and PUSH_CONST.
- Branch-heavy traces introduce control-flow dispatch and shorter basic-block regions.
- The dispatch-step count acts as a simple interpreter overhead proxy.
- This lab demonstrates execution-engine reasoning without claiming compiler, JIT, V8, or WebAssembly implementation experience.
