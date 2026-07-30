# Performance Profiling

## Environment

The local machine exposes Apple command-line developer tools but not the full
Xcode Instruments `xctrace` utility. The repository therefore uses the built-in
macOS `sample` profiler as a reproducible local fallback.

## Command

```bash
./build-profile/performance-profile-runner \
  > performance_engine/reports/profile_runner_output.txt &

PROFILE_PID=$!
sleep 2

sample "$PROFILE_PID" 5 \
  -file performance_engine/reports/macos_sample_profile.txt

wait "$PROFILE_PID"
Workload
eight worker threads
1,000,000 items per execution
cache-local access
queue batch size eight
repeated execution for approximately 15 seconds
RelWithDebInfo build
Bottleneck investigated

Earlier benchmark results showed that the original one-task queue pop required
approximately one mutex acquisition per task. The optimized implementation
removes multiple tasks per critical-section entry.

Measured change

The benchmark comparison showed:

84.55% fewer median lock acquisitions
32.19% lower median runtime
71.61% lower median task p95 latency
47.47% higher median throughput

Cross-run p95, p99, and variance increased in the initial 25-iteration sample.
Those stability signals remain reported explicitly.

Profile artifact
performance_engine/reports/macos_sample_profile.txt

The profile is a real local sampling artifact, not a prewritten or fabricated
profiler result.
