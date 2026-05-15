#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BASELINE = json.loads((ROOT / "runtime_regression_baseline.json").read_text())

def load_json(name):
    return json.loads((ROOT / name).read_text())

checks = []

def add_check(name, passed, detail):
    checks.append({"name": name, "passed": bool(passed), "detail": detail})

# 1. Semantic-equivalence harness
sem = load_json("semantic_equivalence_report.json")
sem_results = sem["results"]
sem_mismatches = [r for r in sem_results if not r["semantically_equivalent"]]

add_check(
    "semantic_equivalence_total_cases",
    len(sem_results) == BASELINE["semantic_equivalence"]["expected_total_cases"],
    f"observed={len(sem_results)}"
)

add_check(
    "semantic_equivalence_expected_mismatch_count",
    len(sem_mismatches) == BASELINE["semantic_equivalence"]["expected_mismatches"],
    f"observed_mismatches={len(sem_mismatches)}"
)

add_check(
    "semantic_equivalence_expected_mismatch_case",
    any(r["case"] == BASELINE["semantic_equivalence"]["expected_mismatch_case"] for r in sem_mismatches),
    f"mismatch_cases={[r['case'] for r in sem_mismatches]}"
)

# 2. Runtime equivalence harness
runtime_eq = load_json("runtime_equivalence_summary.json")

add_check(
    "runtime_equivalence_no_semantic_mismatches",
    runtime_eq["semantic_mismatches"] == BASELINE["runtime_equivalence"]["expected_semantic_mismatches"],
    f"semantic_mismatches={runtime_eq['semantic_mismatches']}"
)

add_check(
    "runtime_equivalence_deopt_fallback_matches",
    runtime_eq["deopt_fallback_matches"] == BASELINE["runtime_equivalence"]["expected_deopt_fallback_matches"],
    f"deopt_fallback_matches={runtime_eq['deopt_fallback_matches']}"
)

add_check(
    "runtime_equivalence_invalid_rejections_preserved",
    runtime_eq["invalid_trace_rejections_preserved"] == BASELINE["runtime_equivalence"]["expected_invalid_trace_rejections_preserved"],
    f"invalid_trace_rejections_preserved={runtime_eq['invalid_trace_rejections_preserved']}"
)

# 3. Wasm malformed regression
wasm = load_json("wasm_regression_summary.json")

add_check(
    "wasm_malformed_total_traces",
    wasm["total_traces"] == BASELINE["wasm_malformed_regression"]["expected_total_traces"],
    f"total_traces={wasm['total_traces']}"
)

add_check(
    "wasm_malformed_no_false_accepts",
    wasm["false_accepts"] == BASELINE["wasm_malformed_regression"]["expected_false_accepts"],
    f"false_accepts={wasm['false_accepts']}"
)

add_check(
    "wasm_malformed_regression_status",
    wasm["regression_status"] == BASELINE["wasm_malformed_regression"]["expected_regression_status"],
    f"regression_status={wasm['regression_status']}"
)

# 4. Tiered execution / deopt
tiered = load_json("speculative_guard_cases.json")
guard_cases = tiered["guard_cases"]
observed_reasons = sorted({
    c["guard_failure_reason"]
    for c in guard_cases
    if c.get("guard_failed") and c.get("guard_failure_reason") != "none"
})
expected_reasons = sorted(BASELINE["tiered_execution"]["expected_deopt_reasons"])

add_check(
    "tiered_execution_guard_failure_reasons",
    observed_reasons == expected_reasons,
    f"observed={observed_reasons}"
)

# 5. Inline-cache terminal states
ic = load_json("property_access_profiles.json")
sites = {s["site"]: s["final_state"] for s in ic["sites"]}

for site, expected_state in BASELINE["inline_cache"]["expected_terminal_states"].items():
    add_check(
        f"inline_cache_terminal_state_{site}",
        sites.get(site) == expected_state,
        f"observed={sites.get(site)} expected={expected_state}"
    )

failed = [c for c in checks if not c["passed"]]

summary = {
    "scope": "CI-style runtime regression gate for toy execution-engine labs",
    "total_runtime_checks": len(checks),
    "checks_passed": len(checks) - len(failed),
    "checks_failed": len(failed),
    "semantic_drift_detected": any(
        (not c["passed"]) and ("semantic" in c["name"] or "equivalence" in c["name"])
        for c in checks
    ),
    "false_accepts": wasm["false_accepts"],
    "classification_regressions": [
        c for c in failed
        if "terminal_state" in c["name"] or "guard_failure_reasons" in c["name"]
    ],
    "regression_gate_status": "PASS" if not failed else "FAIL",
    "checks": checks
}

(ROOT / "runtime_regression_summary.json").write_text(json.dumps(summary, indent=2))

lines = []
lines.append("# Runtime Regression Gate Report\n")
lines.append("> Scope: CI-style regression gate for toy execution-engine labs. This is not production VM, JIT, V8, or WebAssembly validation infrastructure.\n")
lines.append("| Check | Status | Detail |")
lines.append("|---|---|---|")

for c in checks:
    lines.append(f"| {c['name']} | {'PASS' if c['passed'] else 'FAIL'} | {c['detail']} |")

lines.append("\n## Summary\n")
lines.append(f"- Total runtime checks: {summary['total_runtime_checks']}")
lines.append(f"- Checks passed: {summary['checks_passed']}")
lines.append(f"- Checks failed: {summary['checks_failed']}")
lines.append(f"- Semantic drift detected: {str(summary['semantic_drift_detected']).lower()}")
lines.append(f"- False accepts: {summary['false_accepts']}")
lines.append(f"- Regression gate status: `{summary['regression_gate_status']}`")

lines.append("\n## Safe interpretation\n")
lines.append("This gate compares current runtime experiment outputs against checked-in expectations and flags semantic drift, malformed-trace false accepts, deoptimization classification drift, and inline-cache state regressions.")

(ROOT / "runtime_regression_report.md").write_text("\n".join(lines) + "\n")

print(json.dumps({
    "total_runtime_checks": summary["total_runtime_checks"],
    "checks_passed": summary["checks_passed"],
    "checks_failed": summary["checks_failed"],
    "regression_gate_status": summary["regression_gate_status"]
}, indent=2))

if failed:
    raise SystemExit(1)
