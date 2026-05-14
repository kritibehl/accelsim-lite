# FIFO Verification Report

## Summary

This lab adds a small SystemVerilog FIFO block, directed testbench, and assertion-style checks to demonstrate digital verification fundamentals.

## Files

| File | Purpose |
|---|---|
| `rtl/simple_fifo.sv` | FIFO RTL block |
| `tb/fifo_tb.sv` | Directed SystemVerilog testbench |
| `tb/fifo_assertions.sv` | Assertion checks |
| `docs/fifo_verification_plan.md` | Verification plan |
| `docs/fifo_coverage_notes.md` | Scenario coverage notes |

## Tested scenarios

- reset behavior
- normal write/read
- FIFO ordering
- full FIFO condition
- empty FIFO condition
- attempted overfill
- attempted under-read
- simultaneous read/write

## Assertion checks

- count never exceeds depth
- full flag matches count
- empty flag matches count
- read while empty warning
- write while full warning

## Result

Expected result: directed testbench completes with no fatal errors for legal FIFO behavior.

## Scope boundary

This is not a UVM testbench and does not claim production-grade verification signoff.

It demonstrates:
- RTL reading/writing familiarity
- directed testbench construction
- assertion-style thinking
- corner-case verification planning
- FIFO state and boundary-condition reasoning
