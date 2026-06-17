# CAD Flow Automation Lab

## Scope

This module adds hardware-adjacent CAD-flow automation artifacts inside AccelSim-Lite.

It demonstrates:
- RTL lint-style checks
- mock simulation regression summaries
- changed-file impact analysis
- CI-friendly report generation
- lightweight hardware-design validation artifacts

## Designs

- `rtl/fifo.sv`
- `rtl/arbiter.sv`
- `rtl/register_file.sv`

## Automation scripts

- `scripts/run_lint.py`
- `scripts/run_sim.py`
- `scripts/run_regression.py`
- `scripts/summarize_results.py`
- `scripts/changed_rtl_impact.py`

## Reports

- `reports/lint_report.md`
- `reports/simulation_results.json`
- `reports/regression_summary.md`
- `reports/changed_rtl_impact.md`

## Run

From repo root:

```bash
python3 cad_flow/scripts/run_regression.py
Claims boundary

This is an educational CAD-flow automation lab. It does not claim production EDA signoff, physical design, synthesis ownership, or Google internal CAD infrastructure experience.
