#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT_JSON = ROOT / "kv_cache_batching_results.json"
OUT_MD = ROOT / "kv_cache_batching_report.md"

profiles = [
    {"name": "small_batch_decode", "batch": 1, "context": 2048, "decode_steps": 64},
    {"name": "medium_batch_decode", "batch": 8, "context": 4096, "decode_steps": 128},
    {"name": "large_batch_prefill_decode", "batch": 16, "context": 8192, "decode_steps": 256},
    {"name": "queue_pressure_serving", "batch": 32, "context": 4096, "decode_steps": 192}
]

memory_ports = [1, 2, 4, 8]

results = []
for p in profiles:
    for ports in memory_ports:
        kv_pressure = p["batch"] * p["context"] * 0.00008
        decode_dependency = p["decode_steps"] * 0.75
        batching_overhead = p["batch"] * 3.5
        effective_memory_pressure = kv_pressure / ports
        latency = effective_memory_pressure + decode_dependency + batching_overhead
        throughput = (p["batch"] * p["decode_steps"]) / latency

        if effective_memory_pressure > decode_dependency and effective_memory_pressure > batching_overhead:
            bottleneck = "kv-cache-memory-pressure"
        elif decode_dependency > batching_overhead:
            bottleneck = "decode-dependency-pressure"
        else:
            bottleneck = "batching-queue-pressure"

        results.append({
            "profile": p["name"],
            "batch": p["batch"],
            "context": p["context"],
            "decode_steps": p["decode_steps"],
            "memory_ports": ports,
            "simulated_latency": round(latency, 4),
            "simulated_throughput": round(throughput, 6),
            "kv_pressure": round(effective_memory_pressure, 4),
            "decode_dependency": round(decode_dependency, 4),
            "batching_overhead": round(batching_overhead, 4),
            "bottleneck": bottleneck
        })

summary = {}
for p in profiles:
    name = p["name"]
    low = next(r for r in results if r["profile"] == name and r["memory_ports"] == 1)
    high = next(r for r in results if r["profile"] == name and r["memory_ports"] == 8)
    summary[name] = {
        "latency_improvement_1_to_8_ports_percent": round(100 * (low["simulated_latency"] - high["simulated_latency"]) / low["simulated_latency"], 2),
        "throughput_gain_1_to_8_ports_percent": round(100 * (high["simulated_throughput"] - low["simulated_throughput"]) / low["simulated_throughput"], 2),
        "bottleneck_1_port": low["bottleneck"],
        "bottleneck_8_ports": high["bottleneck"]
    }

payload = {
    "scope": "simulated KV-cache/token batching pressure model; not real LLM serving",
    "results": results,
    "summary": summary
}
OUT_JSON.write_text(json.dumps(payload, indent=2))

lines = [
    "# KV-Cache and Token Batching Simulation",
    "",
    "> Scope: simulated inference-systems pressure model. Not real LLM serving or accelerator benchmarking.",
    "",
    "| Profile | Latency improvement 1->8 ports | Throughput gain 1->8 ports | Bottleneck @1 port | Bottleneck @8 ports |",
    "|---|---:|---:|---|---|"
]
for name, s in summary.items():
    lines.append(
        f"| {name} | {s['latency_improvement_1_to_8_ports_percent']}% | "
        f"{s['throughput_gain_1_to_8_ports_percent']}% | {s['bottleneck_1_port']} | {s['bottleneck_8_ports']} |"
    )

lines += [
    "",
    "## Interpretation",
    "",
    "- Larger contexts and batches increase simulated KV-cache pressure.",
    "- Memory-port scaling reduces KV pressure, after which decode dependency can dominate.",
    "- Queue-pressure serving cases show batching overhead as a separate serving constraint.",
    "",
    "## Claims boundary",
    "",
    "This is a synthetic inference-systems model, not a real LLM serving benchmark."
]
OUT_MD.write_text("\n".join(lines) + "\n")

print(f"Generated {OUT_JSON}")
print(f"Generated {OUT_MD}")
