#include "uthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ITERATIONS 1000000

uthread_t main_thread_placeholder;
uthread_t ping_thread;
uthread_t pong_thread;

void ping(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        uthread_yield();
    }
}

void pong(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        uthread_yield();
    }
}

int main(void) {
    struct timeval start, end;
    
    printf("Starting context switch benchmark (%d iterations)...\n", ITERATIONS * 2);
    
    uthread_create(&ping_thread, ping, NULL);
    uthread_create(&pong_thread, pong, NULL);
    
    gettimeofday(&start, NULL);
    
    // Both threads will yield to each other in FCFS/RR, but main thread might also run.
    // If we want ping and pong to strictly interleave, we can just join them and let them yield.
    uthread_join(ping_thread);
    uthread_join(pong_thread);
    
    gettimeofday(&end, NULL);
    
    long long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
    long long total_switches = ITERATIONS * 2;
    double avg_us = (double)elapsed_us / total_switches;
    
    printf("Total time: %lld us\n", elapsed_us);
    printf("Average context switch time: %.3f us\n", avg_us);
    
    return 0;
}
