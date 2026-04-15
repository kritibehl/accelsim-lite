#!/usr/bin/env bash
set -euo pipefail

BIN=./build/accelsim-lite
WORKLOAD=workloads/memory_heavy.csv

echo "=== WHAT-IF: Memory Ports Sweep ==="

for m in 1 2 4
do
  echo "---- memory_ports=$m ----"
  $BIN run $WORKLOAD --memory-ports $m
  echo
done
