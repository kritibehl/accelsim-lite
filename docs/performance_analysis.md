# Performance Analysis

## Scope

This document describes the deterministic performance-engine extension in
AccelSim-Lite. The module compares single-threaded and multithreaded execution,
queue behavior, lock contention, memory-allocation activity, cache access
patterns, workload scaling, compiler optimization, and profiler observations.

The workloads are synthetic and are intended for C++ systems-performance
experimentation. They are not production trading workloads.

## Architecture

The performance engine contains five main elements:

```text
Deterministic data set
        |
        v
Work-item partitioner
        |
        +---------------------------+
        |                           |
        v                           v
Single-thread executor       Shared task queue
                                    |
                         +----------+----------+
                         |          |          |
                         v          v          v
                      Worker 1   Worker 2   Worker N
                         |          |          |
                         +----------+----------+
                                    |
                                    v
                     Metrics and checksum aggregation
                                    |
                                    v
                    JSON, CSV and Markdown reports
