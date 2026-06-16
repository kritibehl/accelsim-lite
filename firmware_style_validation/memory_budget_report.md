# Memory Budget Report

## Scope

Firmware-style memory-budget summary for constrained-runtime validation.

| Component | Memory behavior |
|---|---|
| FixedPointFilter | constant state: alpha + current state |
| Ring buffer | fixed capacity of 8 bytes plus index metadata |
| Hardware mock | bounded register-write history for tests |

## Claims boundary

This validates constrained-memory design patterns, not production embedded memory profiling.
