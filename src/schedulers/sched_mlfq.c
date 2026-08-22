#include "uthread_sched.h"
#include "../internal.h"
#include <stddef.h>

#define NUM_QUEUES 3
#define BOOST_INTERVAL 50

static uthread_tcb_t *queues_head[NUM_QUEUES];
static uthread_tcb_t *queues_tail[NUM_QUEUES];

static int tick_counter = 0;

static void mlfq_init(void) {
    for (int i = 0; i < NUM_QUEUES; i++) {
        queues_head[i] = NULL;
        queues_tail[i] = NULL;
    }
    tick_counter = 0;
}

static void mlfq_enqueue(uthread_tcb_t *tcb) {
    // Safety check
    if (tcb->priority < 0) tcb->priority = 0;
    if (tcb->priority >= NUM_QUEUES) tcb->priority = NUM_QUEUES - 1;
    
    int p = tcb->priority;
    tcb->next = NULL;
    
    if (queues_tail[p] == NULL) {
        queues_head[p] = tcb;
        queues_tail[p] = tcb;
    } else {
        queues_tail[p]->next = tcb;
        queues_tail[p] = tcb;
    }
}

static uthread_tcb_t* mlfq_dequeue(void) {
    for (int i = 0; i < NUM_QUEUES; i++) {
        if (queues_head[i] != NULL) {
            uthread_tcb_t *tcb = queues_head[i];
            queues_head[i] = queues_head[i]->next;
            if (queues_head[i] == NULL) {
                queues_tail[i] = NULL;
            }
            tcb->next = NULL;
            return tcb;
        }
    }
    return NULL;
}

static void mlfq_on_tick(void) {
    // Thread used its time quantum. Demote it if possible.
    if (current_thread && current_thread->id != 0) { // Don't strictly need to avoid demoting main, but good practice
        if (current_thread->priority < NUM_QUEUES - 1) {
            current_thread->priority++;
        }
    }
    
    tick_counter++;
    if (tick_counter >= BOOST_INTERVAL) {
        tick_counter = 0;
        
        // Boost current thread
        if (current_thread) {
            current_thread->priority = 0;
        }
        
        // Move everything from queues 1 and 2 to queue 0
        for (int i = 1; i < NUM_QUEUES; i++) {
            uthread_tcb_t *curr = queues_head[i];
            while (curr != NULL) {
                uthread_tcb_t *next = curr->next;
                curr->priority = 0;
                // We'll just push them into queue 0
                mlfq_enqueue(curr);
                curr = next;
            }
            queues_head[i] = NULL;
            queues_tail[i] = NULL;
        }
    }
    
    uthread_yield();
}

scheduler_ops_t sched_mlfq = {
    .init = mlfq_init,
    .enqueue = mlfq_enqueue,
    .dequeue = mlfq_dequeue,
    .on_tick = mlfq_on_tick
};
