#include "uthread.h"
#include "uthread_mutex.h"
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 10
#define NUM_ITEMS 100

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

uthread_sem_t empty;
uthread_sem_t full;
uthread_mutex_t mutex;

void producer(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_ITEMS; i++) {
        uthread_sem_wait(&empty);
        uthread_mutex_lock(&mutex);
        
        buffer[in] = i;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        
        uthread_mutex_unlock(&mutex);
        uthread_sem_signal(&full);
    }
}

void consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_ITEMS; i++) {
        uthread_sem_wait(&full);
        uthread_mutex_lock(&mutex);
        
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        
        if (item != i) {
            fprintf(stderr, "Consumer expected %d but got %d\n", i, item);
            exit(EXIT_FAILURE);
        }
        
        uthread_mutex_unlock(&mutex);
        uthread_sem_signal(&empty);
    }
}

int main(void) {
    uthread_sem_init(&empty, BUFFER_SIZE);
    uthread_sem_init(&full, 0);
    uthread_mutex_init(&mutex);
    
    uthread_t prod, cons;
    
    printf("Starting producer-consumer test...\n");
    
    uthread_create(&prod, producer, NULL);
    uthread_create(&cons, consumer, NULL);
    
    uthread_join(prod);
    uthread_join(cons);
    
    printf("PASS: producer_consumer completed successfully\n");
    return 0;
}
