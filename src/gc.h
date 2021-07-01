#ifndef GC_H
#define GC_H

#include "state.h"
#include "types.h"
#include "helpers.h"

void *aq_gc_alloc(aq_state_t *aq, size_t amt, uint8_t bit);

#define GC_NEW(aq, type, bit) (CAST(aq_gc_alloc(aq, sizeof(type), bit), type *))

#define GC_NEW_ARRAY(aq, type, nitems, bit)                                    \
    (CAST(aq_gc_alloc(aq, nitems * sizeof(type), bit), type *))

#define GC_NEW_BYTES(aq, bytes, type, bit)                                     \
    (CAST(aq_gc_alloc(aq, bytes, bit), type *))

#endif

