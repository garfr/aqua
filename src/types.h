#ifndef TYPES_H
#define TYPES_H

#include "aqua.h"

typedef struct {
    uint8_t *mem;
    uint8_t *top;
    size_t len;
} aq_heap_t;

/* the "template" for a function, instantiated as a closure */
typedef struct {
    /* debug name */
    const char *name;
    size_t name_sz;

    const uint32_t *code;
    size_t code_sz;
} aq_template_t;

/* an instantiation of a template, ready to be executed with its captured
 * upvalues/enviroment */
typedef struct {
    aq_template_t *t;
} aq_closure_t;

/* a function call record */
typedef struct {
    size_t var_pos;
    aq_closure_t *fn;
} aq_call_t;

struct aq_state_t {
    aq_obj_t *vars;
    size_t vars_sz;

    aq_call_t *calls;
    size_t calls_sz;
    aq_call_t *cur_call;

    aq_alloc_t alloc;
    aq_heap_t heap;

    aq_panic_t panic;
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

typedef enum {
    AQ_OP_RET,
    AQ_OP_MOV,
    AQ_OP_NIL,
    AQ_OP_ADD,
    AQ_OP_SUB,
    AQ_OP_MUL,
    AQ_OP_DIV,
    AQ_OP_CONS,
    AQ_OP_CAR,
    AQ_OP_CDR,
} aq_op_t;

#define ENCODE_ABC(op, a, b, c)                                                \
    ((((uint8_t)(op & 0xFF)) << 24) +                                          \
     (((uint8_t)(a & 0xFF) << 16) + (((uint8_t)(b & 0xFF) << 8)) +             \
      ((uint8_t)(c & 0xFF))))
#endif