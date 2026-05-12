#!/usr/bin/env bash
set -euo pipefail

echo "== Build =="
rm -rf build
mkdir build
cmake -S . -B build
cmake --build build -j

echo
echo "== Validation workloads =="
./scripts/validation/run_validation.sh

echo
echo "== Analysis artifacts =="
python3 scripts/analysis/workload_comparison.py
python3 scripts/analysis/classify_workloads.py
python3 scripts/analysis/memory_sweep_report.py
python3 scripts/analysis/plot_workloads.py
python3 scripts/analysis/plot_memory_sweep.py
python3 scripts/analysis/build_dashboard_report.py
python3 scripts/validation/generate_timing_validation.py

echo
echo "== Claims check =="
python3 scripts/quality/check_claims.py

echo
echo "Quality gate passed."
