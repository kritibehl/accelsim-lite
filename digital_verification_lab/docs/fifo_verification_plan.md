# FIFO Verification Plan

## Scope

This lab verifies a small synchronous FIFO RTL block using a SystemVerilog testbench and assertion-style checks.

This is a learning and portfolio artifact. It does not claim production UVM methodology, full formal verification, or vendor-signoff coverage.

## Design under test

`simple_fifo.sv`

Features:
- parameterized data width
- parameterized depth
- write enable
- read enable
- full flag
- empty flag
- occupancy count
- simultaneous read/write when FIFO is neither full nor empty

## Verification goals

| Goal | Expected behavior |
|---|---|
| Reset behavior | FIFO starts empty with count = 0 |
| Normal write/read | Data is read in FIFO order |
| Full condition | `full` asserts when count reaches depth |
| Empty condition | `empty` asserts when count reaches zero |
| Overfill attempt | Count must not exceed depth |
| Under-read attempt | Count must not go below zero |
| Simultaneous read/write | FIFO accepts concurrent operation when legal |
| State consistency | full/empty flags match count |

## Assertions

Planned assertion checks:
- count never exceeds depth
- empty flag matches zero count
- full flag matches depth count
- no read attempt while empty
- no write attempt while full

## Out of scope

- UVM environment
- constrained random testing
- formal proof
- gate-level timing
- synthesis signoff
- CDC verification
