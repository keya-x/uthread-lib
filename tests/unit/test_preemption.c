#include "uthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

static volatile bool worker_ran = false;
static volatile bool loop_finished = false;

void infinite_loop_thread(void *arg) {
    (void)arg;
    printf("Infinite loop thread started.\n");
    
    // Busy wait loop for a bit, wait for a few timer ticks
    long long volatile counter = 0;
    while (!worker_ran) {
        counter++;
        if (counter > 200000000LL) {
            // Failsafe so the test doesn't actually hang forever if preemption fails
            printf("Preemption failed! Failsafe triggered.\n");
            break;
        }
    }
    
    loop_finished = true;
    printf("Infinite loop thread exiting.\n");
}

void worker_thread(void *arg) {
    (void)arg;
    printf("Worker thread got CPU!\n");
    worker_ran = true;
}

int main(void) {
    uthread_t t1, t2;
    
    printf("Starting test_preemption\n");
    
    // Create the infinite loop thread first so it runs first
    if (uthread_create(&t1, infinite_loop_thread, NULL) != 0) return EXIT_FAILURE;
    
    // Create the worker thread
    if (uthread_create(&t2, worker_thread, NULL) != 0) return EXIT_FAILURE;
    
    // Join the threads. Joining t1 will block the main thread and switch to t1.
    // t1 will run an infinite loop. If preemption works, the timer will interrupt it,
    // call yield, switch to t2, which sets worker_ran to true.
    // Then t1 will see worker_ran == true and exit, unblocking main.
    
    uthread_join(t1);
    uthread_join(t2);
    
    if (worker_ran && loop_finished) {
        printf("PASS: test_preemption completed successfully\n");
        return 0;
    } else {
        printf("FAIL: test_preemption failed\n");
        return EXIT_FAILURE;
    }
}
