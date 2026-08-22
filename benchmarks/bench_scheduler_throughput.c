#include "uthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define NUM_THREADS 500
#define YIELDS_PER_THREAD 1000

void throughput_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < YIELDS_PER_THREAD; i++) {
        uthread_yield();
    }
}

int main(void) {
    uthread_t threads[NUM_THREADS];
    struct timeval start, end;
    
    printf("Starting scheduler throughput benchmark...\n");
    
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_create(&threads[i], throughput_worker, NULL);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_join(threads[i]);
    }
    
    gettimeofday(&end, NULL);
    
    long long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
    long long total_yields = NUM_THREADS * YIELDS_PER_THREAD;
    double throughput = (double)total_yields / (elapsed_us / 1000000.0);
    
    printf("Total time: %lld us\n", elapsed_us);
    printf("Throughput: %.2f operations/sec\n", throughput);
    
    return 0;
}
