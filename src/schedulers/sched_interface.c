#include "uthread_sched.h"
#include <stdlib.h>
#include <string.h>

static scheduler_ops_t *active_scheduler = NULL;
static int initialized = 0;

scheduler_ops_t* get_scheduler(void) {
    if (!initialized) {
        // Fallback or read environment variable
        char *sched_env = getenv("UTHREAD_SCHEDULER");
        if (sched_env) {
            if (strcmp(sched_env, "FCFS") == 0) {
                active_scheduler = &sched_fcfs;
            } else if (strcmp(sched_env, "RR") == 0) {
                active_scheduler = &sched_rr;
            } else if (strcmp(sched_env, "MLFQ") == 0) {
                active_scheduler = &sched_mlfq;
            } else if (strcmp(sched_env, "PRIORITY") == 0) {
                active_scheduler = &sched_priority;
            } else {
                active_scheduler = &sched_fcfs;
            }
        } else {
            active_scheduler = &sched_fcfs;
        }
        
        active_scheduler->init();
        initialized = 1;
    }
    return active_scheduler;
}
