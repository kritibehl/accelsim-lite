#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REPORTS = ROOT / "reports"
OUT = REPORTS / "performance_dashboard.html"

def read_json(path):
    return json.loads(path.read_text())

cache = read_json(ROOT / "performance_systems/cache_locality_results.json")
kv = read_json(ROOT / "performance_systems/kv_cache_batching_results.json")
runtime_gate = read_json(ROOT / "execution_engine_lab/runtime_regression_summary.json")
wasm = read_json(ROOT / "execution_engine_lab/wasm_regression_summary.json")
runtime_eq = read_json(ROOT / "execution_engine_lab/runtime_equivalence_summary.json")

cache_rows = cache["results"]
kv_summary = kv["summary"]

def bar(value, max_value):
    width = 0 if max_value == 0 else min(100, (value / max_value) * 100)
    return f'<div class="bar"><span style="width:{width:.2f}%"></span></div>'

max_cache = max(r["relative_to_contiguous_x"] for r in cache_rows)

cache_table = "\n".join(
    f"""
    <tr>
      <td>{r['experiment']}</td>
      <td>{r['elapsed_ms']}</td>
      <td>{r['relative_to_contiguous_x']}x {bar(r['relative_to_contiguous_x'], max_cache)}</td>
    </tr>
    """
    for r in cache_rows
)

max_kv_gain = max(v["throughput_gain_1_to_8_ports_percent"] for v in kv_summary.values())

kv_table = "\n".join(
    f"""
    <tr>
      <td>{name}</td>
      <td>{v['latency_improvement_1_to_8_ports_percent']}%</td>
      <td>{v['throughput_gain_1_to_8_ports_percent']}% {bar(v['throughput_gain_1_to_8_ports_percent'], max_kv_gain)}</td>
      <td><code>{v['bottleneck_1_port']}</code> → <code>{v['bottleneck_8_ports']}</code></td>
    </tr>
    """
    for name, v in kv_summary.items()
)

html = f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>AccelSim-Lite Performance Dashboard</title>
<style>
body {{
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
  margin: 40px;
  background: #f7f7f8;
  color: #1f2937;
}}
.hero {{
  background: white;
  border-radius: 16px;
  padding: 28px;
  box-shadow: 0 4px 18px rgba(0,0,0,0.08);
  margin-bottom: 24px;
}}
.grid {{
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 16px;
}}
.card {{
  background: white;
  border-radius: 14px;
  padding: 18px;
  box-shadow: 0 2px 12px rgba(0,0,0,0.06);
}}
.metric {{
  font-size: 30px;
  font-weight: 700;
}}
.label {{
  color: #6b7280;
  font-size: 13px;
}}
table {{
  width: 100%;
  border-collapse: collapse;
  background: white;
  border-radius: 12px;
  overflow: hidden;
}}
th, td {{
  text-align: left;
  padding: 12px;
  border-bottom: 1px solid #e5e7eb;
  vertical-align: middle;
}}
th {{
  background: #111827;
  color: white;
}}
.bar {{
  margin-top: 6px;
  width: 100%;
  height: 10px;
  background: #e5e7eb;
  border-radius: 999px;
  overflow: hidden;
}}
.bar span {{
  display: block;
  height: 100%;
  background: #2563eb;
}}
.section {{
  margin-top: 28px;
}}
code {{
  background: #eef2ff;
  padding: 2px 5px;
  border-radius: 5px;
}}
.note {{
  color: #6b7280;
  font-size: 14px;
}}
</style>
</head>
<body>

<div class="hero">
  <h1>AccelSim-Lite Performance Dashboard</h1>
  <p>C++ runtime/performance experimentation platform with workload benchmarks, runtime regression gates, cache/locality studies, and simulated inference-systems pressure models.</p>
  <p class="note">Scope: simulator-derived metrics and educational systems studies. Not hardware benchmarking, production inference measurement, V8 implementation, or production compiler infrastructure.</p>
</div>

<div class="grid">
  <div class="card">
    <div class="metric">39.16%</div>
    <div class="label">memory-heavy latency improvement from 1→4 memory ports</div>
  </div>
  <div class="card">
    <div class="metric">87.03%</div>
    <div class="label">memory-heavy throughput improvement from 1→4 memory ports</div>
  </div>
  <div class="card">
    <div class="metric">{runtime_eq['semantic_mismatches']}</div>
    <div class="label">runtime semantic mismatches</div>
  </div>
  <div class="card">
    <div class="metric">{wasm['false_accepts']}</div>
    <div class="label">Wasm malformed-trace false accepts</div>
  </div>
</div>

<div class="section">
  <h2>Runtime Regression Gate</h2>
  <table>
    <tr><th>Metric</th><th>Value</th></tr>
    <tr><td>Total runtime checks</td><td>{runtime_gate['total_runtime_checks']}</td></tr>
    <tr><td>Checks passed</td><td>{runtime_gate['checks_passed']}</td></tr>
    <tr><td>Checks failed</td><td>{runtime_gate['checks_failed']}</td></tr>
    <tr><td>Semantic drift detected</td><td>{runtime_gate['semantic_drift_detected']}</td></tr>
    <tr><td>False accepts</td><td>{runtime_gate['false_accepts']}</td></tr>
    <tr><td>Status</td><td><strong>{runtime_gate['regression_gate_status']}</strong></td></tr>
  </table>
</div>

<div class="section">
  <h2>Cache / Locality Study</h2>
  <table>
    <tr><th>Experiment</th><th>Elapsed ms</th><th>Relative to contiguous</th></tr>
    {cache_table}
  </table>
</div>

<div class="section">
  <h2>KV-Cache / Token Batching Simulation</h2>
  <table>
    <tr><th>Profile</th><th>Latency improvement</th><th>Throughput gain</th><th>Bottleneck transition</th></tr>
    {kv_table}
  </table>
</div>

</body>
</html>
"""

OUT.write_text(html)
print(f"Generated {OUT}")
