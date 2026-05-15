# WebAssembly-Style Malformed Trace Regression Report

> Scope: toy WebAssembly-style malformed-trace regression workflow. This is not a real Wasm validator, V8 implementation, JIT, or WebAssembly runtime.

| Case | Expected Rejection | Observed Rejection | Match | False Accept |
|---|---|---|---|---|
| stack_underflow_add | stack_underflow | stack_underflow | true | false |
| type_mismatch_add | type_mismatch | type_mismatch | true | false |
| invalid_branch_depth | invalid_branch_depth | invalid_branch_depth | true | false |
| unterminated_block | unterminated_block | unterminated_block | true | false |
| unterminated_loop | unterminated_block | unterminated_block | true | false |
| unexpected_block_end | unknown_opcode | unknown_opcode | true | false |
| malformed_return_shape | stack_underflow | stack_underflow | true | false |

## Summary

- Total traces: 7
- Correctly rejected traces: 7
- Mismatches: 0
- False accepts: 0
- Regression status: `pass`

## Error classes covered

- `invalid_branch_depth`
- `stack_underflow`
- `type_mismatch`
- `unknown_opcode`
- `unterminated_block`

## Safe interpretation

This regression pack validates deterministic rejection behavior for malformed toy Wasm-style traces. It demonstrates validation-regression thinking without claiming production WebAssembly runtime or compiler experience.
