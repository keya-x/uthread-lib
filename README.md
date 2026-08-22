# uthread-lib

`uthread-lib` is a high-performance, POSIX-compliant user-level threading library engineered in C from the ground up. It implements lightweight green threads, context switching, timer-based preemptive scheduling, and atomic-backed synchronization primitives entirely independent of `pthreads` or other third-party abstractions.

This library serves as a robust demonstration of operating system internals and concurrent systems design, offering microsecond-level context switching and supporting workloads of tens of thousands of concurrent threads efficiently.

## Core Capabilities

- **Lightweight Green Threads**: Fast context switching powered by `ucontext.h`, allowing for millions of yields per second without kernel-mode transitions.
- **Preemptive Engine**: Robust time-slicing utilizing `setitimer` (`SIGVTALRM`), protected by meticulous signal masking to guarantee thread safety and prevent re-entrancy anomalies.
- **Pluggable Scheduling Architecture**: Supports interchangeable algorithms configurable at runtime:
  - First-Come-First-Served (FCFS)
  - Round Robin (RR)
  - Multi-Level Feedback Queue (MLFQ)
  - Priority-with-Aging
- **Non-Busy-Waiting Concurrency**: Implements `Mutexes`, `Condition Variables`, and counting `Semaphores`. These utilize foundational atomic compare-and-swap (`stdatomic.h`) Spinlocks merely to protect internal wait-queues, ensuring blocked threads gracefully yield the CPU instead of wasting cycles.

## Benchmark Results

*Measured on an average Linux environment with 2,000,000 context switches and 500,000 yields.*

| Scheduler      | Avg Context Switch Time | Throughput (Yields/sec) | Fairness Profile |
|----------------|-------------------------|--------------------------|------------------|
| **FCFS**       | **~0.40 µs**            | **~2.31 Million**        | Poor (Starves I/O) |
| **RR**         | **~0.39 µs**            | **~2.30 Million**        | Excellent        |
| **MLFQ**       | **~0.41 µs**            | **~2.26 Million**        | Good (Favors I/O)|
| **PRIORITY**   | **~0.39 µs**            | ~157 Thousand            | Good (Aging)     |

Detailed metrics are available in `docs/scheduler_comparison.md`.

## System Design Decisions

- **`ucontext.h` vs Raw Assembly**: `ucontext.h` was chosen over inline assembly to guarantee portability across CPU architectures and strictly adhere to POSIX compliance when preserving signal masks and CPU registers.
- **MLFQ Priority Boosting**: In a Multi-Level Feedback Queue, demoting CPU-bound threads can lead to starvation if new threads arrive frequently. A periodic priority boost moves all threads back to the highest priority queue, ensuring long-running CPU-bound jobs receive guaranteed CPU slices.
- **Mutex Wait Queues**: The foundation of the synchronization primitives is an atomic Spinlock. However, spinlocks busy-wait. The `uthread_mutex_t` limits the spinlock purely to protecting its internal linked list (wait queue). When the mutex is locked, the calling thread is queued and its state transitions to `BLOCKED`, yielding the CPU entirely. It is restored to `READY` immediately upon owner unlock.
- **Signal Masking**: Because the timer signal can interrupt a thread unpredictably, manipulating global scheduler data structures inside `uthread_yield` or `uthread_create` is extremely dangerous. The library explicitly masks `SIGVTALRM` during critical sections to prevent re-entrancy bugs.

## Build and Run Instructions

The project uses a standard `Makefile`. Build the library and run the comprehensive test suite (including race condition resolution, deadlock prevention, stress tests, and benchmarks) with a single command:

```bash
make clean
make
./scripts/run_all_tests.sh
```

To run individual tests under a specific scheduler, use the `UTHREAD_SCHEDULER` environment variable:
```bash
UTHREAD_SCHEDULER=MLFQ ./bin/bench_scheduler_throughput
```

## Demonstrations

- **Race Condition Resolution**: `tests/unit/test_race_condition.c` spawns 5 threads incrementing a shared counter 1000 times each.
  - *Before*: Unsynchronized threads preempt mid-increment, resulting in a final count far below 5000.
  - *After*: Mutex protection guarantees atomic increments, locking the final count reliably at exactly 5000.
- **Deadlock Prevention (Dining Philosophers)**: `tests/unit/test_deadlock_dining_philosophers.c`
  - *Before*: A naive approach where all philosophers seize their left fork simultaneously produces an unrecoverable circular wait deadlock.
  - *After*: The library enforces strict resource ordering (philosophers seize the lower-numbered fork first), permanently dismantling the circular wait condition.

## Future Enhancements at Scale

- **M:N Threading (Multi-Core)**: Evolve the architecture from a 1:N model (one kernel thread running N user threads) to an M:N model by spawning multiple worker `pthreads`, adding run queues per core, and integrating work-stealing algorithms to maximize physical core utilization.
- **Heap-based Priority Queue**: Upgrade the Priority Scheduler's $O(N)$ wait-time scan to an array-based Binary Max-Heap, dropping dequeue complexity to $O(\log N)$ and massively elevating throughput under heavy concurrency.
- **True Non-Blocking I/O**: Intercept and handle blocking system calls (like `read()` or `sleep()`) via `epoll` or `select` within the core scheduler loop to prevent individual blocking threads from halting the underlying kernel thread.

## Known Limitations

- **Single-Core Utilization**: Operates on a single kernel thread and cannot natively distribute load across multiple CPU cores without the proposed M:N threading enhancement.
- **Blocking System Calls**: A thread executing a blocking OS system call will stall the entire application process until the call resolves.
- **Linux/POSIX Exclusive**: Architected around POSIX-specific APIs (`ucontext.h`, `setitimer`), requiring an emulation layer for native Windows execution.
