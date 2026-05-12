# Clock Cycle Breakdown

> Scope: simulator-cycle timing validation, not hardware-cycle prediction.

| Workload | Expected Cycles | Observed Cycles | Drift | Drift % | Avg Latency | Throughput | Top Bottleneck |
|---|---:|---:|---:|---:|---:|---:|---|
| compute_heavy | 18 | 18 | 0 | 0.0% | 10.0000 | 0.3333 | WaitingDependency |
| memory_heavy | 43 | 43 | 0 | 0.0% | 23.8333 | 0.1395 | NoMemoryPort |
| mixed | 30 | 30 | 0 | 0.0% | 16.8333 | 0.2000 | WaitingDependency |
| queue_pressure | 25 | 25 | 0 | 0.0% | 15.0000 | 0.3200 | WaitingDependency |

## Pipeline-stage timing

### compute_heavy

| Stage | Busy Cycles |
|---|---:|
| Decode | 2 |
| Dispatch | 14 |
| Execute | 10 |
| Fetch | 1 |
| Issue | 0 |
| Retire | 5 |

### memory_heavy

| Stage | Busy Cycles |
|---|---:|
| Decode | 2 |
| Dispatch | 19 |
| Execute | 38 |
| Fetch | 1 |
| Issue | 30 |
| Retire | 6 |

### mixed

| Stage | Busy Cycles |
|---|---:|
| Decode | 2 |
| Dispatch | 25 |
| Execute | 24 |
| Fetch | 1 |
| Issue | 8 |
| Retire | 6 |

### queue_pressure

| Stage | Busy Cycles |
|---|---:|
| Decode | 2 |
| Dispatch | 13 |
| Execute | 20 |
| Fetch | 1 |
| Issue | 13 |
| Retire | 4 |

## Stall timing

### compute_heavy

| Stall Reason | Cycles |
|---|---:|
| DispatchQueueFull | 0 |
| NoComputeUnit | 0 |
| NoMemoryPort | 0 |
| ReadyQueueFull | 0 |
| WaitingDependency | 27 |

### memory_heavy

| Stall Reason | Cycles |
|---|---:|
| DispatchQueueFull | 0 |
| NoComputeUnit | 0 |
| NoMemoryPort | 49 |
| ReadyQueueFull | 0 |
| WaitingDependency | 36 |

### mixed

| Stall Reason | Cycles |
|---|---:|
| DispatchQueueFull | 0 |
| NoComputeUnit | 0 |
| NoMemoryPort | 8 |
| ReadyQueueFull | 0 |
| WaitingDependency | 46 |

### queue_pressure

| Stall Reason | Cycles |
|---|---:|
| DispatchQueueFull | 0 |
| NoComputeUnit | 24 |
| NoMemoryPort | 0 |
| ReadyQueueFull | 0 |
| WaitingDependency | 30 |

## Interpretation

- Cycle drift is measured against deterministic simulator baselines.
- A non-zero drift indicates a timing regression in simulator behavior.
- Stall timing helps identify whether latency is dominated by dependency, memory-port, compute, or queue pressure.
