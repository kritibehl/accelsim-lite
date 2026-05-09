# ADR 0001: Workload-level simulator, not cycle-accurate GPU model

## Status

Accepted

## Context

AccelSim-Lite is designed to explain workload-level bottleneck behavior across compute-heavy, memory-heavy, mixed, and queue-pressure workloads.

A full GPU simulator would require modeling warp scheduling, cache-line behavior, Tensor Core timing, SM occupancy, compiler behavior, and vendor-specific runtime details. That is outside the scope of this project.

## Decision

AccelSim-Lite intentionally models workload-level behavior rather than cycle-accurate GPU execution.

It focuses on:

- throughput
- average latency
- queue pressure
- stall classification
- bottleneck transitions
- memory-vs-dependency pressure
- what-if resource sensitivity

## Consequences

This makes the project useful for performance reasoning and interview discussion, but not for exact hardware latency prediction.

Safe claim:

> AccelSim-Lite is a deterministic workload-level performance model for reasoning about throughput, latency, and bottleneck shifts under constrained compute and memory resources.

Unsafe claim:

> AccelSim-Lite is an accurate GPU simulator.
