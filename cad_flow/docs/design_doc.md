# CAD Flow Automation Design Doc

## Problem

Chip-development teams need repeatable automation for RTL checks, simulation regression, changed-file impact analysis, and review artifacts.

## Goals

- Automate lint-style checks.
- Run simulation regressions.
- Generate Markdown and JSON reports.
- Map changed RTL files to impacted tests.
- Support CI review workflows.

## Non-goals

- Not a production ASIC verification environment.
- Not a UVM testbench.
- Not a replacement for commercial EDA tools.
- Not a physical design, timing closure, or signoff flow.

## Architecture

```text
RTL files
   ↓
lint-style checks
   ↓
simulation checks
   ↓
regression runner
   ↓
changed-file impact analysis
   ↓
Markdown / JSON CAD-flow reports
   ↓
CI review artifact
Components
rtl/: small SystemVerilog modules.

tb/: smoke-test testbenches.

scripts/: automation wrappers for lint, simulation, impact analysis, and dashboards.

reports/: generated review artifacts.

yosys/: synthesis-smoke script.

Failure Modes
lint failure

simulation failure

missing impacted tests

malformed report output

changed RTL with no mapped regression

Future Work
deeper Verilator integration

stronger Yosys synthesis checks

waveform artifact collection

coverage-style reporting

real changed-file input from GitHub pull requests

Claims boundary
This is an educational CAD-flow automation design doc. It does not claim production EDA, ASIC verification, or Google internal CAD workflow experience.
