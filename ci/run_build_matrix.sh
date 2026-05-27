#!/usr/bin/env bash
set -euo pipefail

mkdir -p ci
REPORT="ci/build_matrix_report.md"

cat > "$REPORT" <<'MD'
# CMake Build Matrix Report

## Scope

Build-profile automation for AccelSim-Lite C++ runtime/performance tooling.

| Profile | Configure | Build | Tests |
|---|---|---|---|
MD

run_profile() {
  local profile="$1"
  local build_dir="$2"
  shift 2

  echo "== Profile: $profile =="

  if cmake -S . -B "$build_dir" "$@" && \
     cmake --build "$build_dir" -j && \
     ctest --test-dir "$build_dir" --output-on-failure; then
    echo "| $profile | PASS | PASS | PASS |" >> "$REPORT"
  else
    echo "| $profile | FAIL | FAIL | FAIL |" >> "$REPORT"
    exit 1
  fi
}

run_profile "Debug" "build-debug" -DCMAKE_BUILD_TYPE=Debug
run_profile "Release" "build-release" -DCMAKE_BUILD_TYPE=Release
run_profile "ASan" "build-asan" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
run_profile "UBSan" "build-ubsan" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer"
run_profile "WarningsAsErrors" "build-werror" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror"

cat >> "$REPORT" <<'MD'

## Profiles covered

- Debug
- Release
- ASan
- UBSan
- warnings-as-errors

## Claims boundary

This validates build reliability across local CMake profiles. It does not claim production CI ownership or hardware validation infrastructure.
MD

cat "$REPORT"
