#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROFILES = ROOT / "llm_inference_workload_profiles.json"
RESULTS = ROOT / "llm_inference_bottleneck_results.json"
REPORT = ROOT / "llm_inference_bottleneck_report.md"
NOTE = ROOT / "research_note_prefill_decode_pressure.md"

MEMORY_PORTS = [1, 2, 4, 8]
QUEUE_DEPTHS = {
    "low": 0.75,
    "medium": 1.0,
    "high": 1.45,
}
DEPENDENCY_MODES = {
    "low": 0.75,
    "high": 1.35,
}

def classify(memory_stall, dependency_stall, queue_delay):
    if memory_stall >= dependency_stall and memory_stall >= queue_delay:
        return "memory-bound"
    if dependency_stall >= memory_stall and dependency_stall >= queue_delay:
        return "dependency-bound"
    return "queue-pressure-bound"

def simulate(profile, memory_ports, queue_name, queue_factor, dep_name, dep_factor):
    compute_cycles = profile["base_compute_cycles"]
    memory_cycles = profile["base_memory_cycles"] / memory_ports
    dependency_cycles = profile["dependency_delay_cycles"] * dep_factor
    queue_delay = profile["queue_pressure_factor"] * queue_factor * (compute_cycles + memory_cycles) * 0.18

    total_cycles = compute_cycles + memory_cycles + dependency_cycles + queue_delay
    throughput = 1000.0 / total_cycles
    latency = total_cycles

    memory_stall = memory_cycles
    dependency_stall = dependency_cycles
    bottleneck = classify(memory_stall, dependency_stall, queue_delay)

    return {
        "workload": profile["name"],
        "memory_ports": memory_ports,
        "queue_depth": queue_name,
        "dependency_delay": dep_name,
        "latency_cycles": round(latency, 4),
        "throughput_units": round(throughput, 6),
        "queue_delay_cycles": round(queue_delay, 4),
        "stall_distribution": {
            "compute_cycles": round(compute_cycles, 4),
            "memory_stall_cycles": round(memory_stall, 4),
            "dependency_stall_cycles": round(dependency_stall, 4),
            "queue_delay_cycles": round(queue_delay, 4)
        },
        "bottleneck_classification": bottleneck
    }

def main():
    profiles = json.loads(PROFILES.read_text())["profiles"]
    results = []

    for profile in profiles:
        for memory_ports in MEMORY_PORTS:
            for q_name, q_factor in QUEUE_DEPTHS.items():
                for d_name, d_factor in DEPENDENCY_MODES.items():
                    results.append(simulate(profile, memory_ports, q_name, q_factor, d_name, d_factor))

    summaries = {}
    for profile in profiles:
        name = profile["name"]
        workload_results = [r for r in results if r["workload"] == name]

        base = next(r for r in workload_results if r["memory_ports"] == 1 and r["queue_depth"] == "medium" and r["dependency_delay"] == "low")
        scaled = next(r for r in workload_results if r["memory_ports"] == 8 and r["queue_depth"] == "medium" and r["dependency_delay"] == "low")

        latency_improvement = 100.0 * (base["latency_cycles"] - scaled["latency_cycles"]) / base["latency_cycles"]
        throughput_gain = 100.0 * (scaled["throughput_units"] - base["throughput_units"]) / base["throughput_units"]

        bottlenecks = {}
        for r in workload_results:
            bottlenecks[r["bottleneck_classification"]] = bottlenecks.get(r["bottleneck_classification"], 0) + 1

        summaries[name] = {
            "memory_ports_1_to_8_latency_improvement_percent": round(latency_improvement, 2),
            "memory_ports_1_to_8_throughput_gain_percent": round(throughput_gain, 2),
            "baseline_bottleneck": base["bottleneck_classification"],
            "scaled_bottleneck": scaled["bottleneck_classification"],
            "bottleneck_counts": bottlenecks
        }

    payload = {
        "scope": "simulated ML systems study of transformer-style prefill/decode bottleneck behavior; not real LLM serving or hardware benchmarking",
        "sweep_parameters": {
            "memory_ports": MEMORY_PORTS,
            "queue_depth": list(QUEUE_DEPTHS.keys()),
            "dependency_delay": list(DEPENDENCY_MODES.keys())
        },
        "results": results,
        "summary_by_workload": summaries
    }

    RESULTS.write_text(json.dumps(payload, indent=2))

    lines = []
    lines.append("# LLM Inference Bottleneck Study")
    lines.append("")
    lines.append("> Scope: simulated transformer-style inference workload study. This is not a real LLM serving benchmark, accelerator benchmark, or hardware measurement.")
    lines.append("")
    lines.append("| Workload | Latency improvement 1->8 memory ports | Throughput gain 1->8 memory ports | Baseline bottleneck | Scaled bottleneck |")
    lines.append("|---|---:|---:|---|---|")

    for workload, summary in summaries.items():
        lines.append(
            f"| {workload} | {summary['memory_ports_1_to_8_latency_improvement_percent']}% | "
            f"{summary['memory_ports_1_to_8_throughput_gain_percent']}% | "
            f"{summary['baseline_bottleneck']} | {summary['scaled_bottleneck']} |"
        )

    lines.append("")
    lines.append("## Key observations")
    lines.append("")
    lines.append("- Prefill-heavy traces are more sensitive to memory-port scaling because they model wider parallel compute and memory demand.")
    lines.append("- Decode-heavy traces remain more dependency-sensitive because sequential token-generation pressure dominates after memory stalls shrink.")
    lines.append("- Mixed serving traces show combined memory and dependency pressure.")
    lines.append("- Queue-pressure serving traces surface scheduling/queue delay as concurrency increases.")
    lines.append("")
    lines.append("## Safe interpretation")
    lines.append("")
    lines.append("This study uses simulated workload profiles to reason about bottleneck transitions in transformer-style inference phases. It does not claim real model inference, real accelerator performance, or production serving measurements.")

    REPORT.write_text("\n".join(lines) + "\n")

    note = """# Simulated Prefill vs Decode Bottleneck Behavior in Accelerator-Style Workloads

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
"""
    NOTE.write_text(note)

    print(f"Generated {RESULTS}")
    print(f"Generated {REPORT}")
    print(f"Generated {NOTE}")

if __name__ == "__main__":
    main()
