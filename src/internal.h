#ifndef UTHREAD_INTERNAL_H
#define UTHREAD_INTERNAL_H

#include "uthread.h"
#include <stdbool.h>

// Stack management
void* uthread_alloc_stack(size_t size);
void uthread_free_stack(void *stack);

// Context management
void uthread_context_init(ucontext_t *ctx, void *stack, size_t stack_size, void (*func)(void*), void *arg, ucontext_t *link);
void uthread_context_switch(ucontext_t *old_ctx, ucontext_t *new_ctx);

// Thread lifecycle management (internal)
void uthread_cleanup(uthread_tcb_t *tcb);

// Global state
extern uthread_tcb_t *current_thread;
extern ucontext_t main_context;

// Timer and signals
void uthread_timer_init(void);
void uthread_signals_disable(void);
void uthread_signals_enable(void);

#endif // UTHREAD_INTERNAL_H
