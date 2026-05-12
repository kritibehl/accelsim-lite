import json
from pathlib import Path

analysis_dir = Path("reports/analysis")
out_dir = Path("reports/visualization")
out_dir.mkdir(parents=True, exist_ok=True)

comparison = json.loads((analysis_dir / "workload_comparison.json").read_text())
classification = json.loads((analysis_dir / "workload_classification.json").read_text())
memory_sweep = json.loads((analysis_dir / "memory_sweep.json").read_text())

payload = {
    "comparison": comparison,
    "classification": classification,
    "memory_sweep": memory_sweep,
    "pipeline_stages": ["Fetch", "Decode", "Dispatch", "Issue", "Execute", "Retire"],
}

html = """<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>AccelSim-Lite Interactive Pipeline Visualizer</title>
  <style>
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      margin: 36px;
      background: #f7f7f8;
      color: #1f2937;
    }
    h1, h2, h3 {
      margin-bottom: 8px;
    }
    .card {
      background: white;
      border-radius: 14px;
      padding: 20px;
      margin: 18px 0;
      box-shadow: 0 3px 14px rgba(0,0,0,0.08);
    }
    select {
      padding: 8px 10px;
      border-radius: 8px;
      border: 1px solid #bbb;
      font-size: 14px;
    }
    .pipeline {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      margin-top: 14px;
    }
    .stage {
      padding: 12px 16px;
      background: #e8eefc;
      border-radius: 10px;
      font-weight: 700;
      min-width: 90px;
      text-align: center;
      border: 2px solid transparent;
    }
    .stage.hot {
      border-color: #d97706;
      background: #fff3cd;
    }
    .metric {
      display: inline-block;
      padding: 10px 14px;
      border-radius: 10px;
      background: #eef7ee;
      margin: 8px 8px 0 0;
      font-weight: 700;
    }
    .warn {
      background: #fff3cd;
    }
    .bad {
      background: #fde2e2;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 12px;
    }
    table {
      border-collapse: collapse;
      width: 100%;
      margin-top: 12px;
    }
    th, td {
      border-bottom: 1px solid #ddd;
      padding: 10px;
      text-align: left;
      font-size: 14px;
    }
    th {
      background: #fafafa;
    }
    .bar-wrap {
      background: #eee;
      border-radius: 8px;
      overflow: hidden;
      height: 18px;
    }
    .bar {
      height: 18px;
      background: #9db5ff;
      width: 0%;
    }
    code {
      background: #eee;
      padding: 2px 5px;
      border-radius: 4px;
    }
    .small {
      color: #555;
      font-size: 13px;
    }
  </style>
</head>
<body>
  <h1>AccelSim-Lite Interactive Pipeline Visualizer</h1>
  <p>
    Interactive static report for workload-level accelerator pipeline analysis.
    This visualizer uses generated simulator artifacts and does not require a backend.
  </p>

  <div class="card">
    <h2>Select workload</h2>
    <select id="workloadSelect"></select>
    <div id="workloadSummary" class="grid"></div>
  </div>

  <div class="card">
    <h2>Pipeline stages</h2>
    <p class="small">Highlighted stage is inferred from the dominant bottleneck.</p>
    <div id="pipeline" class="pipeline"></div>
  </div>

  <div class="card">
    <h2>Stall breakdown</h2>
    <div id="stallBreakdown"></div>
  </div>

  <div class="card">
    <h2>Workload comparison</h2>
    <table id="comparisonTable">
      <thead>
        <tr>
          <th>Workload</th>
          <th>Throughput</th>
          <th>Latency</th>
          <th>Bottleneck</th>
          <th>Classification</th>
        </tr>
      </thead>
      <tbody></tbody>
    </table>
  </div>

  <div class="card">
    <h2>Memory-port sweep</h2>
    <p>
      Shows how increasing memory ports changes throughput, latency, memory stalls,
      and the dominant bottleneck.
    </p>
    <table id="memorySweepTable">
      <thead>
        <tr>
          <th>Memory Ports</th>
          <th>Throughput</th>
          <th>Latency</th>
          <th>NoMemoryPort Stalls</th>
          <th>Bottleneck</th>
        </tr>
      </thead>
      <tbody></tbody>
    </table>
    <div id="memorySweepSummary"></div>
  </div>

  <div class="card">
    <h2>Engineering interpretation</h2>
    <p>
      The memory-heavy workload starts as memory-bound. Increasing memory ports removes
      <code>NoMemoryPort</code> stalls, but exposes dependency-bound execution.
      This demonstrates the core performance-engineering lesson: optimization shifts
      bottlenecks rather than eliminating them.
    </p>
  </div>

<script id="payload" type="application/json">
PAYLOAD_JSON
</script>

<script>
const data = JSON.parse(document.getElementById("payload").textContent);
const workloads = data.comparison.workloads;
const classMap = data.classification.summary;
const stages = data.pipeline_stages;

function bottleneckStage(bottleneck) {
  if (bottleneck === "NoMemoryPort") return "Issue";
  if (bottleneck === "NoComputeUnit") return "Execute";
  if (bottleneck === "WaitingDependency") return "Dispatch";
  if (bottleneck === "DispatchQueueFull") return "Dispatch";
  if (bottleneck === "ReadyQueueFull") return "Issue";
  return "Execute";
}

function renderSelect() {
  const select = document.getElementById("workloadSelect");
  workloads.forEach((w, idx) => {
    const option = document.createElement("option");
    option.value = idx;
    option.textContent = w.workload;
    select.appendChild(option);
  });
  select.addEventListener("change", () => renderWorkload(workloads[Number(select.value)]));
}

function renderWorkload(w) {
  const summary = document.getElementById("workloadSummary");
  summary.innerHTML = `
    <div class="metric">Throughput: ${w.throughput.toFixed(4)} ops/cycle</div>
    <div class="metric">Latency: ${w.average_latency.toFixed(4)} cycles</div>
    <div class="metric warn">Bottleneck: <code>${w.top_bottleneck}</code></div>
    <div class="metric">Class: ${classMap[w.workload]}</div>
  `;

  const hot = bottleneckStage(w.top_bottleneck);
  const pipeline = document.getElementById("pipeline");
  pipeline.innerHTML = "";
  stages.forEach(stage => {
    const div = document.createElement("div");
    div.className = "stage" + (stage === hot ? " hot" : "");
    div.textContent = stage;
    pipeline.appendChild(div);
  });

  renderStalls(w);
}

function renderStalls(w) {
  const container = document.getElementById("stallBreakdown");
  const stalls = w.stall_counts || {};
  const max = Math.max(...Object.values(stalls), 1);
  let html = "<table><thead><tr><th>Stall reason</th><th>Count</th><th>Relative weight</th></tr></thead><tbody>";
  for (const [reason, count] of Object.entries(stalls)) {
    const pct = Math.round((count / max) * 100);
    html += `
      <tr>
        <td><code>${reason}</code></td>
        <td>${count}</td>
        <td><div class="bar-wrap"><div class="bar" style="width:${pct}%"></div></div></td>
      </tr>
    `;
  }
  html += "</tbody></table>";
  container.innerHTML = html;
}

function renderComparisonTable() {
  const tbody = document.querySelector("#comparisonTable tbody");
  tbody.innerHTML = "";
  workloads.forEach(w => {
    tbody.innerHTML += `
      <tr>
        <td><code>${w.workload}</code></td>
        <td>${w.throughput.toFixed(4)}</td>
        <td>${w.average_latency.toFixed(4)}</td>
        <td><code>${w.top_bottleneck}</code></td>
        <td>${classMap[w.workload]}</td>
      </tr>
    `;
  });
}

function renderMemorySweep() {
  const tbody = document.querySelector("#memorySweepTable tbody");
  tbody.innerHTML = "";

  data.memory_sweep.sweep.forEach(r => {
    tbody.innerHTML += `
      <tr>
        <td>${r.memory_ports}</td>
        <td>${r.throughput.toFixed(4)}</td>
        <td>${r.average_latency.toFixed(4)}</td>
        <td>${r.stall_counts.NoMemoryPort || 0}</td>
        <td><code>${r.top_bottleneck}</code></td>
      </tr>
    `;
  });

  const cmp = data.memory_sweep.comparison_1_to_4;
  document.getElementById("memorySweepSummary").innerHTML = `
    <div class="metric">Latency improvement: ${cmp.latency_improvement_percent}%</div>
    <div class="metric">Throughput improvement: ${cmp.throughput_improvement_percent}%</div>
    <div class="metric warn">Bottleneck shift: <code>${cmp.bottleneck_shift}</code></div>
    <div class="metric bad">NoMemoryPort stall reduction: ${cmp.memory_port_stall_reduction}</div>
  `;
}

renderSelect();
renderComparisonTable();
renderMemorySweep();
renderWorkload(workloads[0]);
</script>
</body>
</html>
"""

html = html.replace("PAYLOAD_JSON", json.dumps(payload, indent=2))
(out_dir / "interactive_pipeline.html").write_text(html)
print("Generated reports/visualization/interactive_pipeline.html")
