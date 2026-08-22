# Uthread Library Design

## Thread State Machine

The library defines exactly four states for a thread:
1. `READY`: The thread is eligible to run and is waiting in a scheduler's queue.
2. `RUNNING`: The thread is currently executing on the CPU. Only one thread can be in this state at any given time.
3. `BLOCKED`: The thread is waiting on a synchronization primitive (Mutex, Semaphore, or Condition Variable) or `uthread_join`, and is not eligible to be scheduled.
4. `TERMINATED`: The thread has finished executing its function or called `uthread_exit`. Its resources (stack) are pending cleanup by a `uthread_join`.

### Legal Transitions
- `READY` → `RUNNING`: The scheduler dequeues the thread and context-switches to it.
- `RUNNING` → `READY`: The running thread calls `uthread_yield` or is preempted by the timer signal.
- `RUNNING` → `BLOCKED`: The thread calls a wait operation on a sync primitive (e.g., `uthread_mutex_lock` when locked) or waits on `uthread_join`.
- `BLOCKED` → `READY`: The thread is woken up by another thread (e.g., via `uthread_mutex_unlock` or `uthread_cond_signal`), and placed back into the scheduler's ready queue.
- `RUNNING` → `TERMINATED`: The thread returns from its main function or explicitly calls `uthread_exit`.

## Thread Control Block (TCB) Layout

Every thread is represented by a `uthread_tcb_t` struct, which contains:
- `id`: A unique integer identifying the thread.
- `context`: A `ucontext_t` object storing the saved CPU registers, stack pointer, and signal mask.
- `stack` / `stack_size`: A pointer to the heap-allocated stack memory and its size, used for execution and eventual cleanup.
- `state`: The current state (`READY`, `RUNNING`, `BLOCKED`, `TERMINATED`).
- `priority`: The base priority level (0 being highest), used by the MLFQ and Priority schedulers.
- `wait_time`: A counter used by the Priority scheduler to implement aging (preventing starvation).
- `quantum_used`: (Unused in the final implementation, as timer ticks directly preempt and demote in `on_tick`).
- `next`: An intrusive linked-list pointer used to link the TCB into various queues (scheduler ready queues or synchronization wait queues) without dynamic allocation overhead.
- `join_waiter`: A pointer to another TCB that is blocked waiting for this thread to terminate via `uthread_join`.

## Context Switch Walkthrough

What happens when a thread calls `uthread_yield()`:
1. **Disable Signals**: The thread first calls `uthread_signals_disable()` to mask `SIGVTALRM`. This prevents the timer from firing while we manipulate global scheduler structures, avoiding re-entrancy bugs.
2. **State Update & Enqueue**: The current thread's state is changed from `RUNNING` to `READY`. It is then passed to `get_scheduler()->enqueue()`, which places it into the appropriate ready queue.
3. **Dequeue Next Thread**: The scheduler's `dequeue()` function is called to select the next `READY` thread. If there are no other threads, the current thread continues running.
4. **Context Switch**: The new thread's state is set to `RUNNING`. We call `uthread_context_switch()`, which uses `swapcontext()` to save the CPU registers into the yielding thread's `context` and load the registers from the new thread's `context`.
5. **Resume**: The CPU now begins executing at the instruction pointer saved in the new thread's context. 
6. **Enable Signals**: Upon resuming (which could be returning from `uthread_yield` or starting fresh from the wrapper function), `uthread_signals_enable()` is called to unmask the timer signal and allow preemption once again.
