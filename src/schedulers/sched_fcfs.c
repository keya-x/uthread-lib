#include "uthread_sched.h"
#include <stddef.h>

static uthread_tcb_t *ready_queue_head = NULL;
static uthread_tcb_t *ready_queue_tail = NULL;

static void fcfs_init(void) {
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
}

static void fcfs_enqueue(uthread_tcb_t *tcb) {
    tcb->next = NULL;
    if (ready_queue_tail == NULL) {
        ready_queue_head = tcb;
        ready_queue_tail = tcb;
    } else {
        ready_queue_tail->next = tcb;
        ready_queue_tail = tcb;
    }
}

static uthread_tcb_t* fcfs_dequeue(void) {
    if (ready_queue_head == NULL) {
        return NULL;
    }
    uthread_tcb_t *tcb = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    if (ready_queue_head == NULL) {
        ready_queue_tail = NULL;
    }
    tcb->next = NULL;
    return tcb;
}

static void fcfs_on_tick(void) {
    // FCFS doesn't preempt on tick
}

scheduler_ops_t sched_fcfs = {
    .init = fcfs_init,
    .enqueue = fcfs_enqueue,
    .dequeue = fcfs_dequeue,
    .on_tick = fcfs_on_tick
};
