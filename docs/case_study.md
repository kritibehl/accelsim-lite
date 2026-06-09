# AccelSim-Lite Case Study

## Problem
Profilers show that a workload is slow, but not always which resource is the actual bottleneck.

## Design
AccelSim-Lite classifies stall reasons and links bottlenecks to concrete optimization recommendations.

## Validation
Regression gate detected a prior +51.22% p95 regression and current optimized run passes build and sanitizer checks.

## Tradeoffs
Simplified simulator model, but useful for explaining bottleneck movement and regression gates.
