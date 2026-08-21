# POSIX pthread Queue Benchmark

## Scope

This module implements a bounded work queue in C using POSIX threading
primitives and compares it against the existing C++ multithreaded
performance engine.

## C primitives used

- `pthread_create`
- `pthread_join`
- `pthread_mutex_t`
- `pthread_cond_t`
- `pthread_mutex_trylock`
- `malloc`
- `calloc`
- `free`
- C structs
- raw pointers

## Architecture

```text
Producer
   |
   v
Bounded C queue
pthread_mutex_t
pthread_cond_t
   |
   +--------+--------+--------+
   |        |        |        |
   v        v        v        v
Worker 1 Worker 2 Worker 3 Worker N
pthread_create / pthread_join
   |
   v
Per-thread metrics
   |
   v
Aggregate benchmark report
