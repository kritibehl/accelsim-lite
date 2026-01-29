# AccelSim-Lite  
Deterministic CPU / Accelerator Functional Simulator (C++)

AccelSim-Lite is a deterministic, instruction-level functional simulator for a simplified CPU / ML-accelerator pipeline.  
It is designed to model instruction flow, scheduling, execution latency and memory behavior with a strong emphasis on correctness, reproducibility and testability.

The simulator is intentionally functional (not cycle-accurate) to focus on system behavior, execution invariants and validation rather than microarchitectural detail.

---

## Motivation
Complex systems frequently fail due to subtle ordering bugs, nondeterministic execution and insufficient validation of execution paths.

AccelSim-Lite was built to:
- Make execution behavior explicit and inspectable
- Enable repeatable debugging through deterministic scheduling
- Bridge hardware-style execution models with software-level testing workflows

---

## Key Features
- Instruction-level functional simulation pipeline  
  Fetch → Decode → Dispatch → Issue → Execute → Done
- Deterministic execution  
  Stable scheduling and tie-breaking with bit-for-bit reproducibility
- Compute and memory modeling  
  Compute units with configurable latency and a fixed-latency memory controller
- Trace-driven simulation  
  CSV instruction input and structured execution trace output
- Test-first design  
  GoogleTest unit tests validating queue semantics, state transitions and determinism across runs

---

## What This Project Demonstrates
This project is designed to highlight:
- C++ ownership and lifecycle discipline
- Deterministic system design and reproducibility guarantees
- Clear separation of concerns across scheduler, compute and memory components
- Validation of system invariants rather than only functional outcomes
- Ability to reason across the hardware–software boundary

---

## Project Structure

accelsim-lite/
├── include/accelsim/ Public interfaces and core data structures
├── src/ Core simulation implementation
├── tests/ GoogleTest unit tests
├── tools/ Trace analysis and helper scripts
├── traces/ Sample instruction traces
├── docs/ Architecture and design notes
└── CMakeLists.txt CMake build configuration


---

## Build
AccelSim-Lite uses CMake for build configuration.

```bash
cmake -S . -B build
cmake --build build -j


