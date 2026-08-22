#include "uthread_mutex.h"
#include "../internal.h"
#include "uthread_sched.h"
#include <stdlib.h>
#include <stdio.h>

void uthread_spinlock_init(uthread_spinlock_t *lock) {
    atomic_flag_clear(&lock->flag);
}

void uthread_spinlock_lock(uthread_spinlock_t *lock) {
    // Disable signals so we don't get preempted while holding a spinlock
    uthread_signals_disable();
    while (atomic_flag_test_and_set(&lock->flag)) {
        // Spin
    }
}

void uthread_spinlock_unlock(uthread_spinlock_t *lock) {
    atomic_flag_clear(&lock->flag);
    uthread_signals_enable();
}

void uthread_mutex_init(uthread_mutex_t *mutex) {
    uthread_spinlock_init(&mutex->guard);
    mutex->locked = 0;
    mutex->owner = -1;
    mutex->head = NULL;
    mutex->tail = NULL;
}

void uthread_mutex_lock(uthread_mutex_t *mutex) {
    uthread_spinlock_lock(&mutex->guard);
    
    if (mutex->locked == 0) {
        mutex->locked = 1;
        mutex->owner = current_thread->id;
        uthread_spinlock_unlock(&mutex->guard);
    } else {
        // Must block
        wait_queue_node_t *node = malloc(sizeof(wait_queue_node_t));
        if (!node) {
            fprintf(stderr, "Failed to allocate wait queue node\n");
            exit(EXIT_FAILURE);
        }
        node->tcb = current_thread;
        node->next = NULL;
        
        if (mutex->tail == NULL) {
            mutex->head = node;
            mutex->tail = node;
        } else {
            mutex->tail->next = node;
            mutex->tail = node;
        }
        
        current_thread->state = UTHREAD_STATE_BLOCKED;
        uthread_tcb_t *prev = current_thread;
        uthread_tcb_t *next = get_scheduler()->dequeue();
        
        if (next == NULL) {
            fprintf(stderr, "Deadlock in uthread_mutex_lock\n");
            exit(EXIT_FAILURE);
        }
        
        next->state = UTHREAD_STATE_RUNNING;
        current_thread = next;
        
        // Before we switch, we must release the spinlock, BUT we must do it carefully
        // so that the next thread can run. 
        // We can't release it after switch because the current thread stops executing!
        // We must clear the flag. However, we also called uthread_signals_disable() in spinlock_lock.
        // We want the new thread to run with signals ENABLED.
        atomic_flag_clear(&mutex->guard);
        
        uthread_context_switch(&prev->context, &next->context);
        
        // When we wake up, we hold the mutex because unlock gave it to us.
        // Wait, signals are disabled when context switch resumes? 
        // No, uthread_context_switch restores the signal mask of the resuming thread.
        // But since we are resuming, the resume mask was whatever it was.
        // Let's ensure signals are enabled after we wake up, although they should be.
        // We don't hold the spinlock anymore.
    }
}

void uthread_mutex_unlock(uthread_mutex_t *mutex) {
    uthread_spinlock_lock(&mutex->guard);
    
    if (mutex->head == NULL) {
        mutex->locked = 0;
        mutex->owner = -1;
    } else {
        // Wake up one thread
        wait_queue_node_t *node = mutex->head;
        mutex->head = node->next;
        if (mutex->head == NULL) {
            mutex->tail = NULL;
        }
        
        uthread_tcb_t *woken = node->tcb;
        free(node);
        
        // The woken thread inherits the lock
        mutex->owner = woken->id;
        
        woken->state = UTHREAD_STATE_READY;
        get_scheduler()->enqueue(woken);
    }
    
    uthread_spinlock_unlock(&mutex->guard);
}
