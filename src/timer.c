#include "internal.h"
#include "uthread_sched.h"
#include <sys/time.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define QUANTUM_USEC 10000 // 10ms quantum

static void timer_handler(int signum) {
    (void)signum;
    // Tell the scheduler a tick happened. The scheduler itself will call uthread_yield if needed.
    get_scheduler()->on_tick();
}

void uthread_timer_init(void) {
    struct sigaction sa;
    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Restart system calls might be needed if we had I/O, but fine for now
    
    if (sigaction(SIGVTALRM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    struct itimerval it;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = QUANTUM_USEC;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = QUANTUM_USEC;
    
    if (setitimer(ITIMER_VIRTUAL, &it, NULL) == -1) {
        perror("setitimer");
        exit(EXIT_FAILURE);
    }
}

void uthread_signals_disable(void) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

void uthread_signals_enable(void) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
}
