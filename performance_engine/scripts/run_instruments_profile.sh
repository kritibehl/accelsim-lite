#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="$ROOT/build-profile"
REPORT_DIR="$ROOT/performance_engine/reports"
TRACE_PATH="$REPORT_DIR/accelsim_time_profiler.trace"

mkdir -p "$REPORT_DIR"

if ! command -v xcrun >/dev/null 2>&1; then
  echo "xcrun was not found. Install Xcode command-line tools."
  exit 1
fi

if ! xcrun xctrace list templates 2>/dev/null \
  | grep -q "Time Profiler"; then
  echo "The Time Profiler Instruments template is unavailable."
  exit 1
fi

rm -rf "$BUILD_DIR"

cmake \
  -S "$ROOT" \
  -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake \
  --build "$BUILD_DIR" \
  --target performance-benchmark \
  -j

rm -rf "$TRACE_PATH"

xcrun xctrace record \
  --template "Time Profiler" \
  --output "$TRACE_PATH" \
  --launch -- \
  "$BUILD_DIR/performance-benchmark" \
  multi \
  4 \
  1000000 \
  unfriendly

cat > "$REPORT_DIR/profiler_report.md" <<REPORT
# Instruments Time Profiler Report

## Command

\`\`\`bash
xcrun xctrace record \\
  --template "Time Profiler" \\
  --output performance_engine/reports/accelsim_time_profiler.trace \\
  --launch -- \\
  build-profile/performance-benchmark multi 4 1000000 unfriendly
\`\`\`

## Artifact

- \`performance_engine/reports/accelsim_time_profiler.trace\`

## Workload

- execution mode: multithreaded
- workers: 4
- workload size: 1,000,000
- access pattern: cache-unfriendly
- build type: RelWithDebInfo

## Review procedure

Open the trace in Instruments and inspect:

- functions with the highest self time
- sorting and temporary-buffer costs
- mutex and condition-variable activity
- worker-thread utilization
- queue wait behavior
- cache-unfriendly processing overhead

## Claims boundary

This trace is local profiler evidence from a deterministic synthetic benchmark.
It is not production trading-system or Citadel infrastructure profiling.
REPORT

echo
echo "Generated:"
echo "  $TRACE_PATH"
echo "  $REPORT_DIR/profiler_report.md"
echo
echo "Open the trace with:"
echo "  open \"$TRACE_PATH\""
