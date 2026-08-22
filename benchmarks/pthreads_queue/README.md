# POSIX pthread Queue Benchmark

## Scope

This module implements a bounded work queue in C using POSIX threading
primitives and compares it with AccelSim-Lite's C++ multithreaded performance
engine under equivalent deterministic workloads.

## C and POSIX primitives

- pthread_create
- pthread_join
- pthread_mutex_t
- pthread_cond_t
- pthread_mutex_trylock
- malloc / calloc / free
- raw pointers
- C structs

## Workload equivalence

The C pthread and C++ std::thread implementations use the same:

- deterministic input generation
- 256-item task chunks
- bounded queue capacity of 1024
- cache-local transformation
- temporary buffer allocation
- per-task sorting
- checksum computation
- workload sizes and worker counts

Matching task counts and deterministic checksums are required before
performance results are considered comparable.

## Metrics

The benchmark measures:

- runtime
- throughput
- p50, p95, and p99 task latency
- allocation count
- worker mutex acquisitions
- contended mutex acquisitions
- contention rate
- mutex wait time
- deterministic checksum

## Validation

The pthread implementation is validated with:

- Release correctness tests
- AddressSanitizer
- ThreadSanitizer
- UndefinedBehaviorSanitizer

## Claims boundary

This demonstrates focused POSIX-threaded C systems programming,
synchronization, manual memory management, and performance measurement.
It does not by itself imply broad C fluency or Darwin kernel experience.
