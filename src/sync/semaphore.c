#include "uthread_mutex.h"

void uthread_sem_init(uthread_sem_t *sem, int count) {
    uthread_mutex_init(&sem->mutex);
    uthread_cond_init(&sem->cond);
    sem->count = count;
}

void uthread_sem_wait(uthread_sem_t *sem) {
    uthread_mutex_lock(&sem->mutex);
    while (sem->count <= 0) {
        uthread_cond_wait(&sem->cond, &sem->mutex);
    }
    sem->count--;
    uthread_mutex_unlock(&sem->mutex);
}

void uthread_sem_signal(uthread_sem_t *sem) {
    uthread_mutex_lock(&sem->mutex);
    sem->count++;
    uthread_cond_signal(&sem->cond);
    uthread_mutex_unlock(&sem->mutex);
}
