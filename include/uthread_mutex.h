#ifndef UTHREAD_MUTEX_H
#define UTHREAD_MUTEX_H

#include <stdatomic.h>
#include "uthread.h"

// Spinlock foundation
typedef struct {
    atomic_flag flag;
} uthread_spinlock_t;

void uthread_spinlock_init(uthread_spinlock_t *lock);
void uthread_spinlock_lock(uthread_spinlock_t *lock);
void uthread_spinlock_unlock(uthread_spinlock_t *lock);

// Wait queue node for threads blocked on a sync primitive
typedef struct wait_queue_node {
    uthread_tcb_t *tcb;
    struct wait_queue_node *next;
} wait_queue_node_t;

// Mutex
typedef struct {
    uthread_spinlock_t guard;
    uthread_t owner;
    int locked;
    wait_queue_node_t *head;
    wait_queue_node_t *tail;
} uthread_mutex_t;

void uthread_mutex_init(uthread_mutex_t *mutex);
void uthread_mutex_lock(uthread_mutex_t *mutex);
void uthread_mutex_unlock(uthread_mutex_t *mutex);

// Condition Variable
typedef struct {
    uthread_spinlock_t guard;
    wait_queue_node_t *head;
    wait_queue_node_t *tail;
} uthread_cond_t;

void uthread_cond_init(uthread_cond_t *cond);
void uthread_cond_wait(uthread_cond_t *cond, uthread_mutex_t *mutex);
void uthread_cond_signal(uthread_cond_t *cond);
void uthread_cond_broadcast(uthread_cond_t *cond);

// Semaphore
typedef struct {
    uthread_mutex_t mutex;
    uthread_cond_t cond;
    int count;
} uthread_sem_t;

void uthread_sem_init(uthread_sem_t *sem, int count);
void uthread_sem_wait(uthread_sem_t *sem);
void uthread_sem_signal(uthread_sem_t *sem);

#endif // UTHREAD_MUTEX_H
