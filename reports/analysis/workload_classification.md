# Workload Classification

| Workload | Classification | Top Bottleneck |
|---|---|---|
| compute_heavy | dependency-bound | WaitingDependency |
| memory_heavy | memory-bound | NoMemoryPort |
| mixed | dependency-bound with memory pressure | WaitingDependency |
| queue_pressure | dependency-bound with compute contention | WaitingDependency |
