#include <stdlib.h>
#include <stdio.h>

#include "state.h"
#include "object.h"

#define HEAP_SZ 65536

struct thing {
    uint8_t thing;
};

aq_state_t *aq_init_state(aq_alloc_t alloc) {
    aq_state_t *aq = alloc(NULL, 0, sizeof(aq_state_t));
    aq->alloc = alloc;
    aq_init_heap(aq, &aq->heap, HEAP_SZ);
    return aq;
}

void aq_deinit_state(aq_state_t *aq) {
    aq_deinit_heap(aq, &aq->heap);
    aq->alloc(aq, sizeof(aq_state_t), 0);
}