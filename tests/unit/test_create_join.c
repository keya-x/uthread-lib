#include "uthread.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 3
#define YIELDS_PER_THREAD 3

static int counter = 0; // shared counter to verify deterministic execution

void thread_func(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < YIELDS_PER_THREAD; i++) {
        printf("Thread %d (yield %d)\n", id, i);
        counter++;
        uthread_yield();
    }
    printf("Thread %d exiting\n", id);
    // return implicitly calls uthread_exit via wrapper
}

int main(void) {
    uthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS];
    
    printf("Starting test_create_join\n");
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i] = i + 1;
        if (uthread_create(&threads[i], thread_func, &thread_args[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i + 1);
            return EXIT_FAILURE;
        }
        printf("Created thread %d\n", i + 1);
    }
    
    // We expect main thread to keep running until it yields or blocks
    // So let's join them one by one. Joining thread 0 will yield to the scheduler!
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("Main thread waiting for thread %d\n", i + 1);
        if (uthread_join(threads[i]) != 0) {
            fprintf(stderr, "Failed to join thread %d\n", i + 1);
            return EXIT_FAILURE;
        }
        printf("Main thread joined thread %d\n", i + 1);
    }
    
    if (counter != NUM_THREADS * YIELDS_PER_THREAD) {
        fprintf(stderr, "FAIL: counter is %d, expected %d\n", counter, NUM_THREADS * YIELDS_PER_THREAD);
        return EXIT_FAILURE;
    }
    
    printf("PASS: test_create_join completed successfully\n");
    return 0;
}
