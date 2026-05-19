#!/usr/bin/env bash
set -euo pipefail

echo "== Build =="
cmake -S . -B build
cmake --build build -j

echo
echo "== Core validation workloads =="
./scripts/run_validation_workloads.sh

echo
echo "== Runtime / execution-engine artifacts =="
./build/stack_bytecode_interpreter
./build/bytecode_optimizer
./build/tiered_execution_sim
./build/object_shape_cache_sim
./build/wasm_style_stack_validator
./build/runtime_equivalence_harness
python3 execution_engine_lab/run_wasm_validation_regression.py
python3 execution_engine_lab/run_runtime_regression_gate.py
./build/dispatch_lookup_structure_benchmark

echo
echo "== ML systems study =="
python3 ml_systems_studies/run_prefill_decode_bottleneck_sweep.py

echo
echo "== Quality gate =="
./scripts/quality/run_quality_gate.sh

echo
echo "Flagship demo completed."
