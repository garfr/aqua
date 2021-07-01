#ifndef GC_H
#define GC_H

#include "state.h"
#include "types.h"
#include "helpers.h"

void aq_init_heap(aq_state_t *aq, aq_heap_t *hp, size_t heap_sz);
void aq_deinit_heap(aq_state_t *aq, aq_heap_t *hp);

void *aq_gc_alloc(aq_state_t *aq, size_t amt);

#define GC_NEW(aq, type) (CAST(aq_gc_alloc(aq, sizeof(type)), type *))

#define GC_NEW_ARRAY(aq, type, nitems)                                         \
    (CAST(aq_gc_alloc(aq, nitems * sizeof(type)), type *))

#define GC_NEW_BYTES(aq, bytes, type) (CAST(aq_gc_alloc(aq, bytes), type *))

#endif

