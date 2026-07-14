# TrajectoryGraph-Lite Benchmark Report

> Scope: deterministic synthetic route/trajectory analysis. This does not claim autonomous-driving production experience.

| Case | Nodes | Edges | Components | Disconnected segments | Distance | Turns | Runtime ms |
|---|---:|---:|---:|---:|---:|---:|---:|
| connected_route | 5 | 4 | 1 | 0 | 4.0 | 1 | 274.0383 |
| disconnected_route | 6 | 3 | 3 | 2 | 3.0 | 3 | 2.9064 |

## What this demonstrates

- graph construction from deterministic trajectory points
- connected-component analysis
- disconnected road-segment detection
- Euclidean edge-distance calculation
- turn-transition analysis
- repeatable C++ benchmark reporting

## Claims boundary

This is a synthetic graph/topology analysis module. It does not claim production mapping, autonomous-driving, road-network, or NVIDIA internal tooling experience.
