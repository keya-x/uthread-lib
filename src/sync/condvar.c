#include "uthread_mutex.h"
#include "../internal.h"
#include "uthread_sched.h"
#include <stdlib.h>
#include <stdio.h>

void uthread_cond_init(uthread_cond_t *cond) {
    uthread_spinlock_init(&cond->guard);
    cond->head = NULL;
    cond->tail = NULL;
}

void uthread_cond_wait(uthread_cond_t *cond, uthread_mutex_t *mutex) {
    uthread_spinlock_lock(&cond->guard);
    
    // Enqueue current thread
    wait_queue_node_t *node = malloc(sizeof(wait_queue_node_t));
    if (!node) {
        fprintf(stderr, "Failed to allocate cond wait queue node\n");
        exit(EXIT_FAILURE);
    }
    node->tcb = current_thread;
    node->next = NULL;
    
    if (cond->tail == NULL) {
        cond->head = node;
        cond->tail = node;
    } else {
        cond->tail->next = node;
        cond->tail = node;
    }
    
    // Release mutex
    uthread_mutex_unlock(mutex);
    
    current_thread->state = UTHREAD_STATE_BLOCKED;
    uthread_tcb_t *prev = current_thread;
    uthread_tcb_t *next = get_scheduler()->dequeue();
    
    if (next == NULL) {
        fprintf(stderr, "Deadlock in uthread_cond_wait\n");
        exit(EXIT_FAILURE);
    }
    
    next->state = UTHREAD_STATE_RUNNING;
    current_thread = next;
    
    // Release cond guard before switching
    atomic_flag_clear(&cond->guard);
    
    uthread_context_switch(&prev->context, &next->context);
    
    // Wake up
    // Must re-acquire the mutex before returning to caller!
    uthread_mutex_lock(mutex);
}

void uthread_cond_signal(uthread_cond_t *cond) {
    uthread_spinlock_lock(&cond->guard);
    
    if (cond->head != NULL) {
        wait_queue_node_t *node = cond->head;
        cond->head = node->next;
        if (cond->head == NULL) {
            cond->tail = NULL;
        }
        
        uthread_tcb_t *woken = node->tcb;
        free(node);
        
        woken->state = UTHREAD_STATE_READY;
        get_scheduler()->enqueue(woken);
    }
    
    uthread_spinlock_unlock(&cond->guard);
}

void uthread_cond_broadcast(uthread_cond_t *cond) {
    uthread_spinlock_lock(&cond->guard);
    
    while (cond->head != NULL) {
        wait_queue_node_t *node = cond->head;
        cond->head = node->next;
        
        uthread_tcb_t *woken = node->tcb;
        free(node);
        
        woken->state = UTHREAD_STATE_READY;
        get_scheduler()->enqueue(woken);
    }
    cond->tail = NULL;
    
    uthread_spinlock_unlock(&cond->guard);
}
