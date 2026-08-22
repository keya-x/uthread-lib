#include "uthread.h"
#include "internal.h"
#include "uthread_sched.h"
#include <stdlib.h>
#include <stdio.h>

#define MAX_THREADS 65536

static uthread_tcb_t *thread_table[MAX_THREADS] = {0};
static int next_thread_id = 1; // 0 is reserved for main thread

uthread_tcb_t *current_thread = NULL;
static uthread_tcb_t main_thread_tcb;

static int system_initialized = 0;

// Initialize the threading system lazily
static void init_system(void) {
    if (system_initialized) return;
    
    main_thread_tcb.id = 0;
    main_thread_tcb.state = UTHREAD_STATE_RUNNING;
    main_thread_tcb.stack = NULL;
    main_thread_tcb.stack_size = 0;
    main_thread_tcb.priority = 0;
    main_thread_tcb.join_waiter = NULL;
    
    current_thread = &main_thread_tcb;
    thread_table[0] = &main_thread_tcb;
    
    // get scheduler will initialize it
    get_scheduler();
    
    uthread_timer_init();
    
    system_initialized = 1;
}

int uthread_create(uthread_t *thread, void (*start_routine)(void *), void *arg) {
    uthread_signals_disable();
    if (!system_initialized) init_system();
    
    if (next_thread_id >= MAX_THREADS) {
        uthread_signals_enable();
        return -1;
    }
    
    uthread_t id = next_thread_id++;
    uthread_tcb_t *tcb = malloc(sizeof(uthread_tcb_t));
    if (!tcb) {
        uthread_signals_enable();
        return -1;
    }
    
    tcb->id = id;
    tcb->state = UTHREAD_STATE_READY;
    tcb->stack_size = UTHREAD_STACK_SIZE;
    tcb->stack = uthread_alloc_stack(tcb->stack_size);
    tcb->priority = 0;
    tcb->wait_time = 0;
    tcb->next = NULL;
    tcb->join_waiter = NULL;
    
    uthread_context_init(&tcb->context, tcb->stack, tcb->stack_size, start_routine, arg, NULL);
    
    thread_table[id] = tcb;
    if (thread) *thread = id;
    
    get_scheduler()->enqueue(tcb);
    
    uthread_signals_enable();
    return 0;
}

void uthread_yield(void) {
    uthread_signals_disable();
    if (!system_initialized) init_system();
    
    uthread_tcb_t *prev = current_thread;
    prev->state = UTHREAD_STATE_READY;
    get_scheduler()->enqueue(prev);
    
    uthread_tcb_t *next = get_scheduler()->dequeue();
    if (next == NULL) {
        prev->state = UTHREAD_STATE_RUNNING;
        uthread_signals_enable();
        return;
    }
    
    next->state = UTHREAD_STATE_RUNNING;
    current_thread = next;
    
    uthread_context_switch(&prev->context, &next->context);
    uthread_signals_enable();
}

void uthread_exit(void) {
    uthread_signals_disable();
    if (!system_initialized) init_system();
    
    uthread_tcb_t *prev = current_thread;
    prev->state = UTHREAD_STATE_TERMINATED;
    
    if (prev->join_waiter != NULL) {
        prev->join_waiter->state = UTHREAD_STATE_READY;
        get_scheduler()->enqueue(prev->join_waiter);
        prev->join_waiter = NULL;
    }
    
    uthread_tcb_t *next = get_scheduler()->dequeue();
    if (next == NULL) {
        exit(0);
    }
    
    next->state = UTHREAD_STATE_RUNNING;
    current_thread = next;
    
    uthread_context_switch(&prev->context, &next->context);
    // Should not reach here
}

int uthread_join(uthread_t thread) {
    uthread_signals_disable();
    if (!system_initialized) init_system();
    if (thread < 0 || thread >= next_thread_id) {
        uthread_signals_enable();
        return -1;
    }
    
    uthread_tcb_t *target = thread_table[thread];
    if (!target) {
        uthread_signals_enable();
        return -1;
    }
    
    if (target->state != UTHREAD_STATE_TERMINATED) {
        target->join_waiter = current_thread;
        current_thread->state = UTHREAD_STATE_BLOCKED;
        
        uthread_tcb_t *prev = current_thread;
        uthread_tcb_t *next = get_scheduler()->dequeue();
        
        if (next == NULL) {
            // Deadlock situation: no other thread to run, but we are blocked.
            fprintf(stderr, "Deadlock in uthread_join\n");
            exit(EXIT_FAILURE);
        }
        
        next->state = UTHREAD_STATE_RUNNING;
        current_thread = next;
        uthread_context_switch(&prev->context, &next->context);
    }
    
    // Cleanup the target thread
    if (target->stack) {
        uthread_free_stack(target->stack);
        target->stack = NULL;
    }
    thread_table[thread] = NULL;
    free(target);
    
    uthread_signals_enable();
    return 0;
}
