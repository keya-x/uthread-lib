#ifndef UTHREAD_H
#define UTHREAD_H

#include <ucontext.h>
#include <stddef.h>

#define UTHREAD_STACK_SIZE (64 * 1024)

typedef int uthread_t;

typedef enum {
    UTHREAD_STATE_READY,
    UTHREAD_STATE_RUNNING,
    UTHREAD_STATE_BLOCKED,
    UTHREAD_STATE_TERMINATED
} uthread_state_t;

typedef struct uthread_tcb {
    uthread_t id;
    ucontext_t context;
    void *stack;
    size_t stack_size;
    uthread_state_t state;
    int priority;           // For Priority/MLFQ schedulers
    int wait_time;          // Counter for aging
    
    struct uthread_tcb *next; // Intrusive list pointer for scheduler queues
    struct uthread_tcb *join_waiter; // Thread waiting for this thread to exit
} uthread_tcb_t;

// API
int uthread_create(uthread_t *thread, void (*start_routine)(void *), void *arg);
void uthread_yield(void);
void uthread_exit(void);
int uthread_join(uthread_t thread);

#endif // UTHREAD_H
