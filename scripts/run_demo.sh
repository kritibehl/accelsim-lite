#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build
cmake --build build -j
./build/accelsim-lite benchmark all
./build/accelsim-lite run workloads/memory_heavy.csv --memory-ports 1
python3 tools/plot_report.py reports/latest
