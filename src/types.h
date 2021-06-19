#ifndef TYPES_H
#define TYPES_H

#include "aqua.h"

typedef struct {
    uint8_t *mem;
    uint8_t *top;
    size_t len;
} aq_heap_t;

struct aq_state_t {
    aq_obj_t *stack;
    aq_alloc_t alloc;
    aq_heap_t heap;
};

typedef enum {
    HEAP_PAIR,
    HEAP_CLOSURE,
    HEAP_ARRAY,
    HEAP_TABLE,
    HEAP_CONTIN,
    HEAP_BIGNUM,
} aq_heap_type_t;

/* items to be included in all heap objects */
#define HEAP_OBJ_HEADER                                                        \
    uint8_t t;                                                                 \
    uint8_t mark

/* so that a structure can point to a generic heap object */
typedef struct {
    HEAP_OBJ_HEADER;
} aq_heap_obj_t;

typedef struct {
    HEAP_OBJ_HEADER;
    aq_obj_t car;
    aq_obj_t cdr;
} aq_pair_t;

#endif