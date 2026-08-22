#ifndef UTHREAD_SCHED_H
#define UTHREAD_SCHED_H

#include "uthread.h"

// The interface that all schedulers must implement
typedef struct {
    void (*init)(void);
    void (*enqueue)(uthread_tcb_t *tcb);
    uthread_tcb_t* (*dequeue)(void);
    void (*on_tick)(void);
} scheduler_ops_t;

// Returns the active scheduler
scheduler_ops_t* get_scheduler(void);

// Individual schedulers
extern scheduler_ops_t sched_fcfs;
extern scheduler_ops_t sched_rr;
extern scheduler_ops_t sched_mlfq;
extern scheduler_ops_t sched_priority;

#endif // UTHREAD_SCHED_H
