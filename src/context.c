#define _XOPEN_SOURCE 700
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>

// Helper to ensure thread exits properly if it returns from its entry function
static void wrapper(void (*func)(void*), void *arg) {
    uthread_signals_enable();
    func(arg);
    uthread_exit();
}

void uthread_context_init(ucontext_t *ctx, void *stack, size_t stack_size, void (*func)(void*), void *arg, ucontext_t *link) {
    if (getcontext(ctx) == -1) {
        perror("getcontext");
        exit(EXIT_FAILURE);
    }
    
    ctx->uc_stack.ss_sp = stack;
    ctx->uc_stack.ss_size = stack_size;
    ctx->uc_stack.ss_flags = 0;
    ctx->uc_link = link; // Can be NULL, we rely on our wrapper calling uthread_exit
    
    // cast arg and func nicely for makecontext
    makecontext(ctx, (void (*)())wrapper, 2, func, arg);
}

void uthread_context_switch(ucontext_t *old_ctx, ucontext_t *new_ctx) {
    if (swapcontext(old_ctx, new_ctx) == -1) {
        perror("swapcontext");
        exit(EXIT_FAILURE);
    }
}
