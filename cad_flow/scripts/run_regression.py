#!/usr/bin/env python3
import subprocess
import sys

steps = [
    ["python3", "cad_flow/scripts/run_lint.py"],
    ["python3", "cad_flow/scripts/run_verilator_lint.py"],
    ["python3", "cad_flow/scripts/run_sim.py"],
    ["python3", "cad_flow/scripts/run_icarus_sim.py"],
    ["python3", "cad_flow/scripts/run_yosys_check.py"],
    ["python3", "cad_flow/scripts/changed_rtl_impact.py"],
    ["python3", "cad_flow/scripts/summarize_results.py"],
    ["python3", "cad_flow/scripts/build_dashboard.py"],
]

for step in steps:
    print("==", " ".join(step))
    result = subprocess.run(step)
    if result.returncode != 0:
        sys.exit(result.returncode)

print("CAD flow regression completed.")
