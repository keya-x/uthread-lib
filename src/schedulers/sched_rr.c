#include "uthread_sched.h"
#include <stddef.h>

static uthread_tcb_t *ready_queue_head = NULL;
static uthread_tcb_t *ready_queue_tail = NULL;

static void rr_init(void) {
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
}

static void rr_enqueue(uthread_tcb_t *tcb) {
    tcb->next = NULL;
    if (ready_queue_tail == NULL) {
        ready_queue_head = tcb;
        ready_queue_tail = tcb;
    } else {
        ready_queue_tail->next = tcb;
        ready_queue_tail = tcb;
    }
}

static uthread_tcb_t* rr_dequeue(void) {
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

static void rr_on_tick(void) {
    // RR always preempts on tick
    uthread_yield();
}

scheduler_ops_t sched_rr = {
    .init = rr_init,
    .enqueue = rr_enqueue,
    .dequeue = rr_dequeue,
    .on_tick = rr_on_tick
};
