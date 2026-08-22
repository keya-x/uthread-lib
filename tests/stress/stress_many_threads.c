#include "uthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define NUM_THREADS 10000

void empty_thread(void *arg) {
    (void)arg;
    // Just exit
}

int main(void) {
    uthread_t *threads = malloc(NUM_THREADS * sizeof(uthread_t));
    if (!threads) return EXIT_FAILURE;
    
    struct timeval start, end;
    
    printf("Starting stress test: spawn %d threads\n", NUM_THREADS);
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        if (uthread_create(&threads[i], empty_thread, NULL) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return EXIT_FAILURE;
        }
    }
    
    gettimeofday(&end, NULL);
    long long spawn_time = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
    
    printf("Spawn time: %lld us (%.2f us/thread)\n", spawn_time, (double)spawn_time / NUM_THREADS);
    printf("Approximate memory: %lu bytes per thread stack (plus TCB overhead)\n", (unsigned long)UTHREAD_STACK_SIZE);
    
    // Join them
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_join(threads[i]);
    }
    
    free(threads);
    printf("PASS: stress_many_threads completed\n");
    return 0;
}
