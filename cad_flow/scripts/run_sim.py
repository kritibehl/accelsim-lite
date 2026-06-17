#!/usr/bin/env python3
import json
from pathlib import Path

OUT = Path("cad_flow/reports/simulation_results.json")

# Educational mock simulation outcomes.
cases = [
    {
        "design": "fifo",
        "test": "push_pop_ordering",
        "status": "pass",
        "cycles": 32
    },
    {
        "design": "fifo",
        "test": "full_empty_boundaries",
        "status": "pass",
        "cycles": 48
    },
    {
        "design": "arbiter",
        "test": "priority_grant",
        "status": "pass",
        "cycles": 12
    },
    {
        "design": "register_file",
        "test": "write_then_read",
        "status": "pass",
        "cycles": 20
    }
]

summary = {
    "scope": "educational mock RTL simulation regression results",
    "tests_run": len(cases),
    "tests_passed": sum(1 for c in cases if c["status"] == "pass"),
    "tests_failed": sum(1 for c in cases if c["status"] != "pass"),
    "status": "pass" if all(c["status"] == "pass" for c in cases) else "fail",
    "cases": cases
}

OUT.write_text(json.dumps(summary, indent=2))
print(json.dumps(summary, indent=2))
