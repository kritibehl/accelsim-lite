# Simulated Prefill vs Decode Bottleneck Behavior in Accelerator-Style Workloads

## Abstract

This note studies simulated transformer-style inference workload behavior in AccelSim-Lite, focusing on prefill-heavy, decode-heavy, mixed-serving, and queue-pressure traces.

## Motivation

Modern inference serving has distinct phases. Prefill-like work can expose memory and parallel compute pressure, while decode-like work can expose sequential dependency and latency sensitivity. This study uses simulated profiles to reason about bottleneck shifts.

## Hypothesis

Prefill-heavy traces should benefit more from memory-port scaling, while decode-heavy traces should remain more dependency-bound after memory pressure is reduced.

## Simulator Setup

The study uses synthetic workload profiles and sweeps memory ports, queue depth, and dependency-delay settings. Results are simulator-derived and are not hardware measurements.

## Workload Definitions

- Prefill-heavy: wider prompt/batch-style phase with higher memory and compute pressure.
- Decode-heavy: stepwise token-generation-style phase with higher dependency pressure.
- Mixed serving trace: prefill burst followed by decode-like work.
- Queue-pressure serving trace: concurrent request pattern with queue buildup.

## Sweep Design

Parameters:
- memory ports: 1, 2, 4, 8
- queue depth: low, medium, high
- dependency delay: low, high

## Results

See `llm_inference_bottleneck_results.json` and `llm_inference_bottleneck_report.md`.

## Bottleneck Transition Analysis

The study records memory-bound, dependency-bound, and queue-pressure-bound classifications as simulated resource constraints shift.

## Limitations

This is not real LLM inference, a real accelerator benchmark, or a production serving benchmark. It is a simulator-based systems study.

## Implications for ML Systems Experimentation

The study demonstrates how simulator-driven workload design can isolate bottleneck transitions and compare prefill-like versus decode-like behavior under controlled resource constraints.

## Next Steps

Future work could connect these synthetic traces to measured profiles from real inference runtimes, while preserving the current claims boundary.
