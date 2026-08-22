# C pthreads vs C++ std::thread Benchmark

> Scope: deterministic synthetic concurrency workloads.

- warmups: 3
- measured iterations: 15

| Implementation | Workers | Workload | Median throughput | Median p95 us | Median p99 us | Contention | Lock acquisitions | Allocations |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| C pthreads | 1 | 10000 | 133333333.33 | 55.05 | 56.00 | 0.0000 | 41 | 40 |
| C++ std::thread | 1 | 10000 | 190323931.33 | 32.57 | 33.24 | 0.0000 | 41 | 40 |
| C pthreads | 2 | 10000 | 74074074.07 | 69.00 | 75.00 | 0.0238 | 42 | 40 |
| C++ std::thread | 2 | 10000 | 104438642.30 | 39.06 | 41.89 | 0.0714 | 42 | 40 |
| C pthreads | 4 | 10000 | 95238095.24 | 31.20 | 34.00 | 0.1136 | 44 | 40 |
| C++ std::thread | 4 | 10000 | 101010101.01 | 22.68 | 30.64 | 0.2045 | 44 | 40 |
| C pthreads | 8 | 10000 | 43859649.12 | 34.10 | 40.10 | 0.2917 | 48 | 40 |
| C++ std::thread | 8 | 10000 | 43282173.80 | 30.18 | 41.70 | 0.2708 | 48 | 40 |
| C pthreads | 1 | 100000 | 186219739.29 | 482.50 | 501.10 | 0.0000 | 392 | 391 |
| C++ std::thread | 1 | 100000 | 354820053.01 | 234.65 | 243.26 | 0.0000 | 392 | 391 |
| C pthreads | 2 | 100000 | 290697674.42 | 279.50 | 290.00 | 0.0000 | 393 | 391 |
| C++ std::thread | 2 | 100000 | 433448341.63 | 158.40 | 164.31 | 0.0280 | 393 | 391 |
| C pthreads | 4 | 100000 | 478468899.52 | 150.00 | 154.10 | 0.0177 | 395 | 391 |
| C++ std::thread | 4 | 100000 | 631412786.11 | 95.25 | 97.55 | 0.3392 | 395 | 391 |
| C pthreads | 8 | 100000 | 172711571.68 | 65.50 | 70.00 | 0.1303 | 399 | 391 |
| C++ std::thread | 8 | 100000 | 264317413.50 | 92.04 | 94.65 | 0.2907 | 399 | 391 |
| C pthreads | 1 | 1000000 | 143698807.30 | 2049.80 | 2095.00 | 0.0325 | 3908 | 3907 |
| C++ std::thread | 1 | 1000000 | 303636103.06 | 955.84 | 976.65 | 0.0312 | 3908 | 3907 |
| C pthreads | 2 | 1000000 | 198925800.68 | 1687.00 | 1749.00 | 0.0814 | 3909 | 3907 |
| C++ std::thread | 2 | 1000000 | 327627160.29 | 961.71 | 970.08 | 0.1832 | 3909 | 3907 |
| C pthreads | 4 | 1000000 | 325520833.33 | 262.00 | 301.00 | 0.1102 | 3911 | 3907 |
| C++ std::thread | 4 | 1000000 | 338371344.28 | 901.11 | 921.32 | 0.2792 | 3911 | 3907 |
| C pthreads | 8 | 1000000 | 191460846.26 | 214.00 | 274.94 | 0.2123 | 3915 | 3907 |
| C++ std::thread | 8 | 1000000 | 267478016.65 | 640.77 | 668.44 | 0.3193 | 3915 | 3907 |

## What this demonstrates

- POSIX thread creation and joining
- pthread mutex synchronization
- condition variables
- bounded work queues
- malloc/free ownership
- C pointers and structs
- contention instrumentation
- deterministic cross-language benchmarking

## Claims boundary

This module demonstrates focused POSIX-threaded C systems work. It does not by itself establish broad C fluency, kernel-development experience, or Darwin runtime implementation experience.
