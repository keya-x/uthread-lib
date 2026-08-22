#include "uthread.h"
#include "uthread_mutex.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 5
#define INCREMENTS 1000

static volatile int broken_counter = 0;
static volatile int fixed_counter = 0;
static uthread_mutex_t mutex;

void broken_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < INCREMENTS; i++) {
        int temp = broken_counter;
        uthread_yield(); // force interleave to guarantee race
        temp++;
        uthread_yield(); // force interleave
        broken_counter = temp;
    }
}

void fixed_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < INCREMENTS; i++) {
        uthread_mutex_lock(&mutex);
        int temp = fixed_counter;
        uthread_yield(); // force interleave
        temp++;
        uthread_yield(); // force interleave
        fixed_counter = temp;
        uthread_mutex_unlock(&mutex);
    }
}

int main(void) {
    uthread_t threads[NUM_THREADS];
    
    printf("Running broken race condition demo...\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_create(&threads[i], broken_worker, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_join(threads[i]);
    }
    printf("Broken counter: %d (Expected: %d)\n", broken_counter, NUM_THREADS * INCREMENTS);
    if (broken_counter == NUM_THREADS * INCREMENTS) {
        printf("WARN: Broken counter actually got correct value (rare!)\n");
    }
    
    printf("\nRunning fixed race condition demo...\n");
    uthread_mutex_init(&mutex);
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_create(&threads[i], fixed_worker, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_join(threads[i]);
    }
    printf("Fixed counter: %d (Expected: %d)\n", fixed_counter, NUM_THREADS * INCREMENTS);
    
    if (fixed_counter != NUM_THREADS * INCREMENTS) {
        printf("FAIL: Fixed counter got wrong value!\n");
        return EXIT_FAILURE;
    }
    
    printf("PASS: test_race_condition completed successfully\n");
    return 0;
}
