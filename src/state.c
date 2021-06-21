#include <stdlib.h>
#include <stdio.h>

#include "state.h"
#include "object.h"

#define HEAP_SZ 65536
#define VARS_SZ 1024
#define CALLS_SZ 1024

struct thing {
    uint8_t thing;
};

aq_state_t *aq_init_state(aq_alloc_t alloc) {
    aq_state_t *aq = alloc(NULL, 0, sizeof(aq_state_t));
    aq->alloc = alloc;
    aq_init_heap(aq, &aq->heap, HEAP_SZ);

    aq->vars = alloc(NULL, 0, sizeof(aq_obj_t) * VARS_SZ);
    aq->vars_sz = VARS_SZ;
    for (size_t i = 0; i < aq->vars_sz; i++) {
        aq->vars[i] = OBJ_NIL_VAL;
    }

    aq->calls = alloc(NULL, 0, sizeof(aq_call_t) * CALLS_SZ);
    aq->calls_sz = CALLS_SZ;
    aq->cur_call = aq->calls;

    return aq;
}

void aq_deinit_state(aq_state_t *aq) {
    aq->alloc(aq->vars, sizeof(aq_obj_t) * aq->vars_sz, 0);
    aq->alloc(aq->calls, sizeof(aq_call_t) * aq->calls_sz, 0);
    aq_deinit_heap(aq, &aq->heap);
    aq->alloc(aq, sizeof(aq_state_t), 0);
}