# Benchmark Results

## Context Switch Benchmark
(Measured using 2,000,000 context switches between two threads yielding to each other)

| Scheduler      | Total Time (us) | Avg Context Switch (us) |
|----------------|-----------------|-------------------------|
| **FCFS**       | 803,642         | 0.402                   |
| **RR**         | 783,105         | 0.392                   |
| **MLFQ**       | 827,398         | 0.414                   |
| **PRIORITY**   | 783,879         | 0.392                   |

## Scheduler Throughput Benchmark
(Measured using 500 threads yielding 1000 times each = 500,000 total yields)

| Scheduler      | Total Time (us) | Throughput (ops/sec)    |
|----------------|-----------------|-------------------------|
| **FCFS**       | 216,230         | 2,312,352.59            |
| **RR**         | 217,194         | 2,302,089.38            |
| **MLFQ**       | 220,868         | 2,263,795.57            |
| **PRIORITY**   | 3,171,368       |   157,660.67            |

## Analysis

- **Context Switch Time**: All schedulers achieve roughly similar context switch overhead (~0.4 µs). This validates that the core `swapcontext` overhead is consistent, regardless of scheduler queue manipulation logic (since enqueueing is generally O(1) across the board for this exact benchmark).
- **Scheduler Throughput**: FCFS, RR, and MLFQ are all extremely fast (~2.3M operations/sec), due to O(1) `enqueue` and `dequeue` operations. However, the Priority scheduler drops significantly in throughput (~157K ops/sec). This is expected because our priority scheduler uses an O(N) scan of the ready queue during `dequeue` to find the thread with the highest effective priority. With 500 threads yielding repeatedly, this O(N) cost dominates the runtime.
