import subprocess

commands = [
    ["python3", "scripts/analysis/workload_comparison.py"],
    ["python3", "scripts/analysis/classify_workloads.py"],
    ["python3", "scripts/analysis/memory_sweep_report.py"],
]

passed = 0

for cmd in commands:
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd)

    if result.returncode == 0:
        passed += 1

print(f"\nPassed suites: {passed}/{len(commands)}")
