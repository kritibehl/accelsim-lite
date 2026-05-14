# FIFO Coverage Notes

## Scenario coverage matrix

| Scenario | Covered | Artifact |
|---|---:|---|
| Reset behavior | Yes | `fifo_tb.sv` |
| Single write/read | Yes | `fifo_tb.sv` |
| FIFO ordering | Yes | `fifo_tb.sv` |
| Full FIFO | Yes | `fifo_tb.sv` |
| Empty FIFO | Yes | `fifo_tb.sv` |
| Attempted overfill | Yes | `fifo_tb.sv`, `fifo_assertions.sv` |
| Attempted under-read | Yes | `fifo_tb.sv`, `fifo_assertions.sv` |
| Simultaneous read/write | Yes | `fifo_tb.sv` |
| Count consistency | Yes | `fifo_assertions.sv` |
| Flag consistency | Yes | `fifo_assertions.sv` |

## Functional coverage summary

The testbench covers deterministic directed scenarios rather than randomized coverage.

Covered behavior:
- legal writes
- legal reads
- FIFO order preservation
- full and empty state transitions
- illegal access attempts
- simple concurrent read/write case

Not covered:
- random traffic generation
- full cross coverage
- UVM coverage collection
- formal property proof
- synthesis or timing closure
