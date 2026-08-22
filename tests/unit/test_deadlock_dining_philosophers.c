#include "uthread.h"
#include "uthread_mutex.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_PHILOSOPHERS 5
#define NUM_MEALS 5

uthread_mutex_t forks[NUM_PHILOSOPHERS];

static volatile int meals_eaten[NUM_PHILOSOPHERS] = {0};

void naive_philosopher(void *arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    
    // We only try one meal because it will deadlock
    printf("Naive Philosopher %d wants left fork %d\n", id, left);
    uthread_mutex_lock(&forks[left]);
    printf("Naive Philosopher %d got left fork %d\n", id, left);
    
    // Yield to guarantee all philosophers get their left fork, causing deadlock
    uthread_yield();
    
    printf("Naive Philosopher %d wants right fork %d\n", id, right);
    uthread_mutex_lock(&forks[right]);
    
    printf("Naive Philosopher %d is eating\n", id);
    meals_eaten[id]++;
    
    uthread_mutex_unlock(&forks[right]);
    uthread_mutex_unlock(&forks[left]);
}

void fixed_philosopher(void *arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    
    // Resource ordering strategy: always pick up lower numbered fork first
    int first = (left < right) ? left : right;
    int second = (left < right) ? right : left;
    
    for (int i = 0; i < NUM_MEALS; i++) {
        uthread_mutex_lock(&forks[first]);
        uthread_yield(); // try to induce deadlock if it was broken
        uthread_mutex_lock(&forks[second]);
        
        meals_eaten[id]++;
        
        uthread_mutex_unlock(&forks[second]);
        uthread_mutex_unlock(&forks[first]);
    }
}

int main(int argc, char *argv[]) {
    // If we run this normally, naive deadlocks and hangs the test suite.
    // To make it scriptable, we can accept an argument to run naive vs fixed.
    // Or we can just run fixed. But we must include naive code.
    // Let's run fixed by default, and naive if passed "naive".
    
    int run_naive = (argc > 1 && argv[1][0] == 'n');
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        uthread_mutex_init(&forks[i]);
    }
    
    uthread_t threads[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];
    
    if (run_naive) {
        printf("Running naive dining philosophers (EXPECT DEADLOCK)...\n");
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            ids[i] = i;
            uthread_create(&threads[i], naive_philosopher, &ids[i]);
        }
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            uthread_join(threads[i]);
        }
        printf("If you see this, it didn't deadlock!\n");
    } else {
        printf("Running fixed dining philosophers...\n");
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            ids[i] = i;
            uthread_create(&threads[i], fixed_philosopher, &ids[i]);
        }
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            uthread_join(threads[i]);
        }
        
        int total = 0;
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            total += meals_eaten[i];
        }
        if (total != NUM_PHILOSOPHERS * NUM_MEALS) {
            printf("FAIL: Not all meals were eaten\n");
            return EXIT_FAILURE;
        }
        printf("PASS: All philosophers finished eating.\n");
    }
    
    return 0;
}
