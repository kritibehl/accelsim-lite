# WebAssembly-Style Stack Validation Report

> Scope: toy WebAssembly-style stack validation for structured control-flow traces. This is not a real WebAssembly runtime, compiler, or validator.

| Case | Valid | Error Classification | Steps Checked |
|---|---|---|---:|
| valid_arithmetic_return | true | none | 4 |
| valid_block_branch | true | none | 5 |
| valid_loop_counter_shape | true | none | 7 |
| invalid_stack_underflow_add | false | stack_underflow | 2 |
| invalid_type_mismatch_add | false | type_mismatch | 3 |
| invalid_branch_depth | false | invalid_branch_depth | 3 |
| unterminated_block | false | unterminated_block | 5 |

## Stack-height and control-flow transitions

### valid_arithmetic_return

| PC | Opcode | Stack Before | Stack After | Control Before | Control After | OK | Error |
|---:|---|---:|---:|---:|---:|---|---|
| 0 | i32.const | 0 | 1 | 0 | 0 | true | none |
| 1 | i32.const | 1 | 2 | 0 | 0 | true | none |
| 2 | i32.add | 2 | 1 | 0 | 0 | true | none |
| 3 | return | 1 | 0 | 0 | 0 | true | none |

### valid_block_branch

| PC | Opcode | Stack Before | Stack After | Control Before | Control After | OK | Error |
|---:|---|---:|---:|---:|---:|---|---|
| 0 | block | 0 | 0 | 0 | 1 | true | none |
| 1 | i32.const | 0 | 1 | 1 | 1 | true | none |
| 2 | br_if | 1 | 0 | 1 | 1 | true | none |
| 3 | i32.const | 0 | 1 | 1 | 1 | true | none |
| 4 | return | 1 | 0 | 1 | 0 | true | none |

### valid_loop_counter_shape

| PC | Opcode | Stack Before | Stack After | Control Before | Control After | OK | Error |
|---:|---|---:|---:|---:|---:|---|---|
| 0 | loop | 0 | 0 | 0 | 1 | true | none |
| 1 | local.get | 0 | 1 | 1 | 1 | true | none |
| 2 | i32.const | 1 | 2 | 1 | 1 | true | none |
| 3 | i32.add | 2 | 1 | 1 | 1 | true | none |
| 4 | br_if | 1 | 0 | 1 | 1 | true | none |
| 5 | i32.const | 0 | 1 | 1 | 1 | true | none |
| 6 | return | 1 | 0 | 1 | 0 | true | none |

### invalid_stack_underflow_add

| PC | Opcode | Stack Before | Stack After | Control Before | Control After | OK | Error |
|---:|---|---:|---:|---:|---:|---|---|
| 0 | i32.const | 0 | 1 | 0 | 0 | true | none |
| 1 | i32.add | 1 | 1 | 0 | 0 | false | stack_underflow |

### invalid_type_mismatch_add

| PC | Opcode | Stack Before | Stack After | Control Before | Control After | OK | Error |
|---:|---|---:|---:|---:|---:|---|---|
| 0 | local.get | 0 | 1 | 0 | 0 | true | none |
| 1 | i32.const | 1 | 2 | 0 | 0 | true | none |
| 2 | i32.add | 2 | 0 | 0 | 0 | false | type_mismatch |

### invalid_branch_depth

| PC | Opcode | Stack Before | Stack After | Control Before | Control After | OK | Error |
|---:|---|---:|---:|---:|---:|---|---|
| 0 | block | 0 | 0 | 0 | 1 | true | none |
| 1 | i32.const | 0 | 1 | 1 | 1 | true | none |
| 2 | br_if | 1 | 0 | 1 | 1 | false | invalid_branch_depth |

### unterminated_block

| PC | Opcode | Stack Before | Stack After | Control Before | Control After | OK | Error |
|---:|---|---:|---:|---:|---:|---|---|
| 0 | block | 0 | 0 | 0 | 1 | true | none |
| 1 | i32.const | 0 | 1 | 1 | 1 | true | none |
| 2 | i32.const | 1 | 2 | 1 | 1 | true | none |
| 3 | i32.add | 2 | 1 | 1 | 1 | true | none |
| 4 | end_of_program | 1 | 1 | 1 | 1 | false | unterminated_block |

## Summary

- Valid traces: 3
- Invalid traces: 4
- Error classes observed:
  - `invalid_branch_depth`: 1
  - `stack_underflow`: 1
  - `type_mismatch`: 1
  - `unterminated_block`: 1

## Interpretation

- Operand-stack validation catches stack underflow before execution.
- Type validation catches non-i32 values used by i32 arithmetic.
- Structured control-flow validation catches invalid branch depths and unterminated blocks.
- This lab demonstrates WebAssembly-adjacent validation reasoning without claiming real Wasm engine implementation.
