# AccelSim-Lite Coding Standards

## Goals

The project prioritizes:
- deterministic behavior
- reproducible analysis
- maintainable C++ code
- explicit performance-report generation

## C++ guidelines

- Prefer small focused functions
- Avoid hidden mutable global state
- Keep pipeline stages logically isolated
- Use descriptive enum/state names
- Separate workload parsing from execution logic
- Keep analysis/reporting scripts outside core simulator logic

## Testing

GoogleTest cases should validate:
- invalid configurations
- queue-pressure behavior
- dependency stalls
- latency regressions
- workload boundary conditions

## Claims boundary

The project is a workload-level performance model and intentionally does not claim cycle-accurate GPU execution.
