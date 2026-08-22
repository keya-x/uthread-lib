# Scheduler Comparison

This document details the differences in fairness and throughput observed between the four implemented schedulers: First-Come-First-Served (FCFS), Round Robin (RR), Multi-Level Feedback Queue (MLFQ), and Priority-with-Aging.

## Workload Characterization

We stress-tested the schedulers using two primary workloads:
1. **Scheduler Fairness (`stress_scheduler_fairness`)**: Consists of a CPU-bound thread that loops continuously without yielding voluntarily, alongside an I/O-bound thread that yields frequently.
2. **Scheduler Throughput (`bench_scheduler_throughput`)**: Consists of 500 threads repeatedly yielding 1000 times each, maximizing enqueue/dequeue operations.

## FCFS (First-Come-First-Served)
- **Fairness**: Very Poor. As demonstrated in our fairness stress test, a CPU-bound thread completely monopolizes the CPU under FCFS (602M ticks vs 0 ticks for the I/O-bound thread). Because FCFS does not preempt based on time, threads can starve other ready threads indefinitely.
- **Throughput**: Excellent (~2.3M ops/sec). Because there is no priority logic and enqueueing/dequeueing is strictly $O(1)$ at the tail/head of a single linked list, throughput is extremely high.

## RR (Round Robin)
- **Fairness**: Excellent. Our fairness test showed RR successfully balancing execution time by forcefully preempting the CPU-bound thread every 10ms.
- **Throughput**: Excellent (~2.3M ops/sec). Similar to FCFS, the RR queue is a simple FIFO linked list, resulting in minimal overhead ($O(1)$ operations).

## MLFQ (Multi-Level Feedback Queue)
- **Fairness**: Good. It effectively favors the I/O-bound thread (which yields frequently, staying in the high priority queue) over the CPU-bound thread (which exhausts its quantum and gets demoted to lower queues). The periodic priority boost ensures the CPU-bound thread does not starve permanently.
- **Throughput**: Excellent (~2.26M ops/sec). Although there are multiple queues, the scheduler only checks a fixed number of queues (3) in order, making it an $O(1)$ operation to enqueue and dequeue. The slight dip compared to FCFS/RR is due to the extra queue checking and priority demotion logic.

## Priority Scheduling with Aging
- **Fairness**: Good. Starvation is prevented because waiting threads accumulate "wait time", increasing their effective priority until they eventually preempt a higher-priority CPU-bound thread.
- **Throughput**: Poor (~157K ops/sec). Enqueue operations are $O(1)$ (appending to the tail), but dequeue operations require an $O(N)$ full scan of the ready queue to find the thread with the highest `priority + wait_time`. With many active threads, this scanning cost scales poorly compared to the $O(1)$ schedulers. At scale, a heap-based priority queue would be necessary to achieve $O(\log N)$ performance.
