#include <stdlib.h>
#include "internal.h"

void* uthread_alloc_stack(size_t size) {
    void *stack = malloc(size);
    if (!stack) {
        // According to requirements, fail loudly if a stack cannot be allocated.
        exit(EXIT_FAILURE);
    }
    return stack;
}

void uthread_free_stack(void *stack) {
    free(stack);
}
