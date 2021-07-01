#include <stdlib.h>
#include <stdio.h>

#include "gc.h"

#define HEAP_ALIGN 32

#define ALIGN(num) ((num) + HEAP_ALIGN - ((num) % HEAP_ALIGN))

void *aq_gc_alloc(aq_state_t *aq, size_t amt, uint8_t bit) {
    aq_heap_obj_t *ret = aq->alloc(NULL, 0, amt);
    if (ret == NULL) {
        aq_panic(aq, AQ_ERR_OOM);
    }

    ret->bit = bit;
    ret->gc_forward = aq->gc_root;
    aq->gc_root = ret;
    return (void *)ret;
}
