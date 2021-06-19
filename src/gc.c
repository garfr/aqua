#include <stdlib.h>
#include <stdio.h>

#include "gc.h"

#define HEAP_ALIGN 16

#define ALIGN(num) ((num) + ((num) % HEAP_ALIGN))

void aq_init_heap(aq_state_t *aq, aq_heap_t *hp, size_t heap_sz) {
    hp->mem = aq->alloc(NULL, 0, heap_sz);
    hp->len = heap_sz;
    hp->top = hp->mem;
}

void aq_deinit_heap(aq_state_t *aq, aq_heap_t *hp) {
    aq->alloc(hp->mem, hp->len, 0);
}

void *aq_gc_alloc(aq_heap_t *hp, size_t amt) {
    if ((size_t)((hp->top + amt) - hp->mem) >= hp->len) {
        printf("out of memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    uint8_t *ret = hp->top;
    hp->top = (uint8_t *)ALIGN((size_t)(amt + hp->top));
    return ret;
}
