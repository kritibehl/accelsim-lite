#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build
cmake --build build -j

mkdir -p os_concepts

./build/threading_race_sim | tee os_concepts/threading_race_output.txt
./build/memory_pressure_sim | tee os_concepts/memory_pressure_output.csv

cat > os_concepts/scheduler_latency_report.md <<'MD'
# OS Concepts Scheduler / Latency Report

## Scope

Small C++ OS-concepts validation demos for threading, memory-pressure behavior, and systems-performance discussion.

## Artifacts

- `threading_race_sim.cpp`
- `memory_pressure_sim.cpp`
- `threading_race_output.txt`
- `memory_pressure_output.csv`

## What this demonstrates

- thread contention and synchronization correctness
- unsafe counter behavior versus mutex/atomic protected counters
- memory-pressure-style access over growing working sets
- latency/throughput measurement methodology
- CMake + GoogleTest-backed regression workflow

## Claims boundary

This is an educational OS-concepts validation demo. It does not claim production OS scheduler implementation, kernel work, or hardware memory profiling.
MD

echo "Generated os_concepts/scheduler_latency_report.md"
