#!/usr/bin/env bash
set -euo pipefail

mkdir -p reports/flagship

cmake -S . -B build
cmake --build build -j

ctest --test-dir build --output-on-failure \
  | tee reports/flagship/gtest_report.txt

cat > reports/flagship/gtest_report.md <<'MD'
# GoogleTest / CTest Report

## Scope

This report records the local CTest/GoogleTest execution for AccelSim-Lite.

## Command

```bash
ctest --test-dir build --output-on-failure
Raw output

See:

reports/flagship/gtest_report.txt
Test coverage themes
boundary workload behavior
invalid pipeline configuration
zero-resource configuration
queue pressure
large queue behavior
determinism checks
MD
echo "Generated reports/flagship/gtest_report.md"
