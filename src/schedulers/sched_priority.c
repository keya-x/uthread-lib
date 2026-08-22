#include "uthread_sched.h"
#include <stddef.h>

static uthread_tcb_t *ready_queue_head = NULL;

static void priority_init(void) {
    ready_queue_head = NULL;
}

static void priority_enqueue(uthread_tcb_t *tcb) {
    tcb->wait_time = 0; // reset aging when enqueued
    tcb->next = NULL;
    if (ready_queue_head == NULL) {
        ready_queue_head = tcb;
    } else {
        // Just append to the end, we search on dequeue
        uthread_tcb_t *curr = ready_queue_head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = tcb;
    }
}

static uthread_tcb_t* priority_dequeue(void) {
    if (ready_queue_head == NULL) {
        return NULL;
    }
    
    uthread_tcb_t *prev = NULL;
    uthread_tcb_t *curr = ready_queue_head;
    
    uthread_tcb_t *best_prev = NULL;
    uthread_tcb_t *best = curr;
    int max_prio = best->priority + best->wait_time;
    
    while (curr != NULL) {
        int prio = curr->priority + curr->wait_time;
        if (prio > max_prio) {
            max_prio = prio;
            best = curr;
            best_prev = prev;
        }
        prev = curr;
        curr = curr->next;
    }
    
    if (best_prev == NULL) {
        ready_queue_head = best->next;
    } else {
        best_prev->next = best->next;
    }
    best->next = NULL;
    return best;
}

static void priority_on_tick(void) {
    // Age all threads in the ready queue
    uthread_tcb_t *curr = ready_queue_head;
    while (curr != NULL) {
        curr->wait_time++;
        curr = curr->next;
    }
    // Preempt current thread so we can pick highest priority again
    uthread_yield();
}

scheduler_ops_t sched_priority = {
    .init = priority_init,
    .enqueue = priority_enqueue,
    .dequeue = priority_dequeue,
    .on_tick = priority_on_tick
};
