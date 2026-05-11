# CLI Workflow

## Configure build

```bash
cmake -S . -B build
Build simulator
cmake --build build
Run tests
ctest --test-dir build
Run workload analysis
python3 scripts/analysis/workload_comparison.py
Run memory sweep
python3 scripts/analysis/memory_sweep_report.py
Run automation suite
python3 scripts/automation/run_replay_suite.py
Generate test report
python3 scripts/automation/generate_test_report.py
Run quality gate
./scripts/quality/run_quality_gate.sh

