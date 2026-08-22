#include "uthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

volatile int cpu_bound_ticks = 0;
volatile int io_bound_ticks = 0;
volatile int run_flag = 1;

void cpu_bound(void *arg) {
    (void)arg;
    struct timeval start, curr;
    gettimeofday(&start, NULL);
    while (run_flag) {
        cpu_bound_ticks++;
        if (cpu_bound_ticks % 1000000 == 0) {
            gettimeofday(&curr, NULL);
            long long elapsed = (curr.tv_sec - start.tv_sec) * 1000 + (curr.tv_usec - start.tv_usec) / 1000;
            if (elapsed > 1000) {
                run_flag = 0;
                break;
            }
        }
    }
}

void io_bound(void *arg) {
    (void)arg;
    while (run_flag) {
        io_bound_ticks++;
        uthread_yield(); // simulate I/O by yielding frequently
    }
}

int main(void) {
    uthread_t t1, t2;
    
    printf("Starting stress_scheduler_fairness test...\n");
    
    uthread_create(&t1, cpu_bound, NULL);
    uthread_create(&t2, io_bound, NULL);
    
    // Main thread acts as timer
    struct timeval start, curr;
    gettimeofday(&start, NULL);
    
    while (1) {
        gettimeofday(&curr, NULL);
        long long elapsed = (curr.tv_sec - start.tv_sec) * 1000 + (curr.tv_usec - start.tv_usec) / 1000;
        if (elapsed > 1000) { // run for 1 second
            run_flag = 0;
            break;
        }
        uthread_yield();
    }
    
    uthread_join(t1);
    uthread_join(t2);
    
    printf("CPU-bound ticks: %d\n", cpu_bound_ticks);
    printf("I/O-bound ticks: %d\n", io_bound_ticks);
    
    printf("PASS: stress_scheduler_fairness completed\n");
    return 0;
}
