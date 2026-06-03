# Build Health Report

> Scope: synthetic CI/build analytics over AccelSim-Lite build-history data.

## Summary

```json
{
  "failed_builds": 7,
  "flaky_tests": 2,
  "regression_detected": true,
  "root_cause": "cache invalidation"
}
```

## Build health metrics

- Total builds analyzed: 12
- Failed builds: 7
- Failure rate: 58.33%
- Average pass duration: 426.0 sec
- Average fail duration: 643.57 sec
- Failure-duration delta: 217.57 sec

## Flaky / recurring tests

- `test_metadata_lookup` failed 4 times across builds B-002, B-005, B-009, B-012
- `test_cache_locality` failed 2 times across builds B-007, B-009

## Root-cause trend

- Top recurring root cause: `cache invalidation` (4 occurrences)

## What this means

The build-history data shows recurring test failures and a dominant cache-invalidation root cause. This models CI/build-health workflows used to identify regressions, flaky tests, and recurring infrastructure failure modes.

## Claims boundary

This is a synthetic build-analytics workflow for CI/CD validation practice. It does not claim production build-system ownership or Apple internal infrastructure experience.
