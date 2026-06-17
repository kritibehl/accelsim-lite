# CAD Flow Dashboard

```json
{
  "lint_status": "pass",
  "lint_failures": 0,
  "simulation_status": "pass",
  "simulation_tests": 4,
  "simulation_passed": 4,
  "impacted_tests": 5,
  "verilator_status": "pass",
  "icarus_status": "pass",
  "yosys_status": "pass"
}
```

| Area | Status / Value |
|---|---|
| RTL lint | pass (0 failures) |
| Mock simulation | pass (4/4 passing) |
| Changed-file impact | 5 impacted tests |
| Verilator lint | pass |
| Icarus simulation | pass |
| Yosys check | pass |

## What this means

This dashboard gives a CI-review summary for RTL lint-style checks, simulation regression, changed-file impact analysis, and synthesis-smoke checks.

## Claims boundary

This is an educational CAD-flow dashboard. It does not claim production EDA dashboarding or signoff ownership.
