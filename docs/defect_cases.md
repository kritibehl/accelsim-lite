# Defect Cases

## Invalid workload configuration

Cases:
- zero memory ports
- negative compute units
- invalid queue depth

Expected:
- validation failure
- safe simulator exit

## Queue-pressure overflow

Cases:
- ready queue saturation
- dispatch queue saturation

Expected:
- queue-pressure bottleneck classification

## Dependency stalls

Cases:
- workload dependencies blocking execution

Expected:
- `WaitingDependency` bottleneck classification

## Latency regression

Cases:
- increased average latency after workload/config change

Expected:
- regression surfaced in comparison report
