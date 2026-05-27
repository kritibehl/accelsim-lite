#!/usr/bin/env bash
set -euo pipefail

mkdir -p ci

REPORT="ci/quality_gate_report.md"

{
  echo "# CI Quality Gate Report"
  echo
  echo "## Scope"
  echo
  echo "Jenkins-style quality gate for AccelSim-Lite C++ runtime/performance workflows."
  echo
  echo "## Results"
  echo
} > "$REPORT"

run_step() {
  local name="$1"
  shift

  echo "== $name =="

  if "$@"; then
    echo "- PASS: $name" >> "$REPORT"
  else
    echo "- FAIL: $name" >> "$REPORT"
    exit 1
  fi
}

run_step "Configure CMake" cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
run_step "Build" cmake --build build -j
run_step "CTest" ctest --test-dir build --output-on-failure
run_step "Lookup benchmark" ./build/dispatch_lookup_structure_benchmark
run_step "Cache/locality study" python3 performance_systems/cache_locality_experiment.py
run_step "KV-cache study" python3 performance_systems/kv_cache_batching_sim.py
run_step "Threading benchmark" python3 performance_systems/threading_microbenchmark.py

echo >> "$REPORT"
echo "## Final Status" >> "$REPORT"
echo >> "$REPORT"
echo "PASS" >> "$REPORT"

cat "$REPORT"
