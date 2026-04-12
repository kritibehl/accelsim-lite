#!/usr/bin/env bash
set -euo pipefail

BIN=./build/accelsim-lite

if [ ! -x "$BIN" ]; then
  echo "Binary not found at $BIN"
  echo "Build first with:"
  echo "  rm -rf build && mkdir build && cd build && cmake .. && make -j"
  exit 1
fi

echo "=== Running validation workloads ==="

for w in compute_heavy memory_heavy mixed queue_pressure
do
    echo "---- $w ----"
    $BIN run workloads/$w.csv
    echo
done

echo "=== Benchmark sweep ==="
$BIN benchmark all || true

echo
echo "=== Latest report ==="
$BIN report latest || true
