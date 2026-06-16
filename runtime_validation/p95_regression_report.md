# p95 Regression Report

AccelSim-Lite includes baseline-vs-candidate p95 comparison for runtime/performance experiments.

Example:

```json
{
  "benchmark": "metadata_lookup",
  "baseline_p95_ns": 410,
  "candidate_p95_ns": 620,
  "regression": "+51.22%",
  "decision": "fail"
}
