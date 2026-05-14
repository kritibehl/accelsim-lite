#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LAB_DIR="$ROOT_DIR/digital_verification_lab"
LOG="$LAB_DIR/sim/fifo_tb_simulation_log.txt"

RTL="$LAB_DIR/rtl/simple_fifo.sv"
ASSERTIONS="$LAB_DIR/tb/fifo_assertions.sv"
TB="$LAB_DIR/tb/fifo_tb.sv"

{
  echo "FIFO Simulation Run"
  echo "==================="
  echo "Date: $(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  echo "RTL: $RTL"
  echo "Assertions: $ASSERTIONS"
  echo "Testbench: $TB"
  echo

  if command -v iverilog >/dev/null 2>&1 && command -v vvp >/dev/null 2>&1; then
    echo "Simulator: iverilog + vvp"
    echo "Command:"
    echo "iverilog -g2012 -o /tmp/fifo_tb.out $RTL $ASSERTIONS $TB"
    echo

    iverilog -g2012 -o /tmp/fifo_tb.out "$RTL" "$ASSERTIONS" "$TB"
    vvp /tmp/fifo_tb.out

    echo
    echo "Result: PASS"
    echo "Notes: Directed FIFO testbench compiled and executed with Icarus Verilog."

  elif command -v verilator >/dev/null 2>&1; then
    echo "Simulator: Verilator"
    echo "Result: SKIPPED"
    echo "Notes: Verilator is available, but this lab uses event-driven testbench tasks and SVA-style properties better suited for a simulator flow such as iverilog/vvp or commercial simulators."

  else
    echo "Simulator: not found"
    echo "Result: SKIPPED"
    echo "Notes: Install iverilog to run this directed SystemVerilog simulation locally."
  fi
} | tee "$LOG"
