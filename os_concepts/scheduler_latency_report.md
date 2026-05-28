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
