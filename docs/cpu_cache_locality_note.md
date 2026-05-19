# CPU Cache and Locality Notes

## Purpose

This note documents how AccelSim-Lite thinks about memory behavior at a systems level.

The simulator currently models memory pressure through memory-port limits, queue delay, and synthetic communication pressure. It does not model real CPU cache hierarchy, cache lines, replacement policies, prefetching, NUMA, or hardware counters.

## Current locality-adjacent signals

AccelSim-Lite includes:
- memory-heavy workload classes
- memory-port scaling sweeps
- queue-pressure workloads
- metadata lookup benchmarks using hash-table and tree-backed structures
- workload bottleneck classification

## Future cache/locality extension

A future extension could add:
- contiguous vs pointer-heavy access traces
- simulated cache-hit/cache-miss counters
- structure-of-arrays vs array-of-structures comparisons
- cache-line-sized synthetic blocks
- locality-aware queue traversal experiments

## Claims boundary

This project does not claim hardware cache measurement or CPU microarchitecture benchmarking.
