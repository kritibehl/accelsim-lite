import csv
import json
from pathlib import Path

ANALYSIS_DIR = Path("reports/analysis")
OUT_DIR = Path("reports/visualization")
OUT_DIR.mkdir(parents=True, exist_ok=True)

comparison = json.loads((ANALYSIS_DIR / "workload_comparison.json").read_text())
classification = json.loads((ANALYSIS_DIR / "workload_classification.json").read_text())
memory_sweep = json.loads((ANALYSIS_DIR / "memory_sweep.json").read_text())

class_map = classification["summary"]

pipeline_stages = ["Fetch", "Decode", "Dispatch", "Issue", "Execute", "Retire"]

rows = comparison["workloads"]

html = f"""<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>AccelSim-Lite Workload Visualizer</title>
  <style>
    body {{
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      margin: 40px;
      background: #f7f7f7;
      color: #222;
    }}
    h1, h2 {{
      margin-bottom: 8px;
    }}
    .card {{
      background: white;
      border-radius: 12px;
      padding: 20px;
      margin: 18px 0;
      box-shadow: 0 2px 10px rgba(0,0,0,0.08);
    }}
    table {{
      border-collapse: collapse;
      width: 100%;
      margin-top: 12px;
    }}
    th, td {{
      border-bottom: 1px solid #ddd;
      text-align: left;
      padding: 10px;
      font-size: 14px;
    }}
    th {{
      background: #fafafa;
    }}
    .pipeline {{
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      margin-top: 12px;
    }}
    .stage {{
      padding: 12px 16px;
      background: #e9eefc;
      border-radius: 10px;
      font-weight: 600;
    }}
    .metric {{
      display: inline-block;
      background: #eef7ee;
      padding: 10px 14px;
      border-radius: 8px;
      margin: 6px 8px 6px 0;
      font-weight: 600;
    }}
    .warn {{
      background: #fff3cd;
    }}
    .bad {{
      background: #fde2e2;
    }}
    img {{
      max-width: 100%;
      border-radius: 10px;
      border: 1px solid #ddd;
      margin-top: 10px;
      background: white;
    }}
    code {{
      background: #eee;
      padding: 2px 5px;
      border-radius: 4px;
    }}
  </style>
</head>
<body>
  <h1>AccelSim-Lite Workload Visualizer</h1>
  <p>
    Static visual report for workload-level accelerator pipeline analysis.
    This is not a cycle-accurate GPU simulator; it visualizes throughput, latency,
    queue pressure, and bottleneck shifts from reproducible simulator reports.
  </p>

  <div class="card">
    <h2>Pipeline model</h2>
    <div class="pipeline">
      {''.join(f'<div class="stage">{s}</div>' for s in pipeline_stages)}
    </div>
  </div>

  <div class="card">
    <h2>Workload comparison</h2>
    <table>
      <tr>
        <th>Workload</th>
        <th>Throughput</th>
        <th>Avg Latency</th>
        <th>Top Bottleneck</th>
        <th>Classification</th>
      </tr>
"""

for r in rows:
    html += f"""
      <tr>
        <td><code>{r['workload']}</code></td>
        <td>{r['throughput']:.4f}</td>
        <td>{r['average_latency']:.4f}</td>
        <td><code>{r['top_bottleneck']}</code></td>
        <td>{class_map[r['workload']]}</td>
      </tr>
"""

derived = comparison["derived_metrics"]
sweep = memory_sweep["comparison_1_to_4"]

html += f"""
    </table>
  </div>

  <div class="card">
    <h2>Derived metrics</h2>
    <span class="metric">Throughput range: {derived['throughput_range_ops_per_cycle'][0]}–{derived['throughput_range_ops_per_cycle'][1]} ops/cycle</span>
    <span class="metric">Latency range: {derived['latency_range_cycles'][0]}–{derived['latency_range_cycles'][1]} cycles</span>
    <span class="metric warn">Memory-heavy throughput drop: {derived['memory_vs_compute_throughput_drop_percent']}%</span>
    <span class="metric warn">Memory-heavy latency increase: {derived['memory_vs_compute_latency_increase_x']}x</span>
  </div>

  <div class="card">
    <h2>Memory-port what-if sweep</h2>
    <span class="metric">Latency improvement: {sweep['latency_improvement_percent']}%</span>
    <span class="metric">Throughput improvement: {sweep['throughput_improvement_percent']}%</span>
    <span class="metric warn">Bottleneck shift: <code>{sweep['bottleneck_shift']}</code></span>
    <span class="metric bad">NoMemoryPort stall reduction: {sweep['memory_port_stall_reduction']}</span>

    <table>
      <tr>
        <th>Memory Ports</th>
        <th>Throughput</th>
        <th>Avg Latency</th>
        <th>Top Bottleneck</th>
        <th>NoMemoryPort Stalls</th>
      </tr>
"""

for r in memory_sweep["sweep"]:
    html += f"""
      <tr>
        <td>{r['memory_ports']}</td>
        <td>{r['throughput']:.4f}</td>
        <td>{r['average_latency']:.4f}</td>
        <td><code>{r['top_bottleneck']}</code></td>
        <td>{r['stall_counts'].get('NoMemoryPort', 0)}</td>
      </tr>
"""

html += """
    </table>
  </div>

  <div class="card">
    <h2>Charts</h2>
    <h3>Throughput by workload</h3>
    <img src="../analysis/throughput_by_workload.png" alt="Throughput by workload">

    <h3>Latency by workload</h3>
    <img src="../analysis/latency_by_workload.png" alt="Latency by workload">

    <h3>Memory sweep latency</h3>
    <img src="../analysis/memory_sweep_latency.png" alt="Memory sweep latency">

    <h3>Memory sweep throughput</h3>
    <img src="../analysis/memory_sweep_throughput.png" alt="Memory sweep throughput">

    <h3>Memory stalls vs memory ports</h3>
    <img src="../analysis/memory_sweep_stalls.png" alt="Memory stalls vs memory ports">
  </div>

  <div class="card">
    <h2>Interpretation</h2>
    <p>
      The simulator shows that increasing memory ports removes the memory bottleneck
      on memory-heavy workloads, but exposes the next limiting factor:
      dependency-bound execution. This mirrors real performance engineering:
      optimization shifts bottlenecks rather than eliminating them.
    </p>
  </div>
</body>
</html>
"""

(OUT_DIR / "workload_visualizer.html").write_text(html)
print("Generated reports/visualization/workload_visualizer.html")
