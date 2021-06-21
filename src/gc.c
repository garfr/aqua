#include <stdlib.h>
#include <stdio.h>

#include "gc.h"

#define HEAP_ALIGN 32

#define ALIGN(num) ((num) + HEAP_ALIGN - ((num) % HEAP_ALIGN))

void aq_init_heap(aq_state_t *aq, aq_heap_t *hp, size_t heap_sz) {
    hp->mem = aq->alloc(NULL, 0, heap_sz);
    hp->len = heap_sz;
    hp->top = hp->mem;
}

void aq_deinit_heap(aq_state_t *aq, aq_heap_t *hp) {
    aq->alloc(hp->mem, hp->len, 0);
}

void *aq_gc_alloc(aq_state_t *aq, size_t amt) {
    aq_heap_t *hp = &aq->heap;
    if ((size_t)((hp->top + amt) - hp->mem) >= hp->len) {
        aq_panic(aq);
    }
    uint8_t *ret = (uint8_t *)ALIGN((size_t)hp->top);
    hp->top = ret + amt;
    return ret;
}
