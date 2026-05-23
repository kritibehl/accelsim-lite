# Sanitizers and Profiling Workflow

## Scope

This document describes reproducible C++ quality and profiling workflows for AccelSim-Lite.

## Sanitizer builds

AddressSanitizer / UndefinedBehaviorSanitizer style build:

```bash
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"

cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
Profiling workflows
macOS Instruments

Recommended for local macOS profiling:

Time Profiler
Allocations
System Trace
Linux perf

On Linux:

perf record ./build/accelsim-lite examples/workloads/memory_heavy.csv
perf report
Valgrind / cachegrind

On Linux:

valgrind --tool=cachegrind ./build/accelsim-lite examples/workloads/memory_heavy.csv
Claims boundary

These workflows document profiling and sanitizer setup. They do not claim production performance tuning or hardware-counter benchmarking unless those commands are actually run and results are saved.
