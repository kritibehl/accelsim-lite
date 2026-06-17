# CAD Flow Automation Design Doc

## Problem

Chip-development teams need repeatable automation for RTL checks, simulation regression, changed-file impact analysis, and review artifacts.

## Goals

- Automate lint-style checks
- Run simulation regressions
- Generate Markdown and JSON reports
- Map changed RTL files to impacted tests
- Support CI review workflows

## Non-Goals

- Not a production ASIC verification environment
- Not a UVM testbench
- Not a replacement for commercial EDA tools
- Not a physical-design or timing-closure flow

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
rtl/

Small SystemVerilog modules:

fifo
arbiter
register file
tb/

Smoke-test validation benches.

scripts/

Automation wrappers for:

lint
simulation
impact analysis
dashboards
reports/

Generated review artifacts and CI outputs.

yosys/

Synthesis-smoke validation scripts.

Failure Modes
lint failure
simulation failure
missing impacted tests
malformed reports
unmapped RTL changes
Future Work
Verilator integration
stronger Yosys checks
waveform collection
coverage reporting
PR-based impact analysis
Claims Boundary

Educational CAD-flow automation example only.
