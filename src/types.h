#ifndef TYPES_H
#define TYPES_H

#include "aqua.h"

/* items to be included in all heap objects */
#define HEAP_OBJ_HEADER                                                        \
    uint8_t tt;                                                                \
    uint8_t mark

typedef struct {
    uint8_t *mem;
    uint8_t *top;
    size_t len;
} aq_heap_t;

/* the "template" for a function, instantiated as a closure */
typedef struct {
    HEAP_OBJ_HEADER;
    /* debug name */
    const char *name;
    size_t name_sz;

    const uint32_t *code;
    size_t code_sz;

    const aq_obj_t *lits;
    size_t lits_sz;
} aq_template_t;

/* an instantiation of a template, ready to be executed with its captured
 * upvalues/enviroment */
typedef struct {
    HEAP_OBJ_HEADER;
    aq_template_t *t;
} aq_closure_t;

typedef struct aq_sym_t {
    HEAP_OBJ_HEADER;
    struct aq_sym_t *next;
    size_t l;
    char s[1];
} aq_sym_t;

typedef struct {
    size_t syms;
    size_t buckets_sz;
    aq_sym_t **buckets;
} aq_sym_tbl_t;

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

    aq_sym_tbl_t syms;

    aq_panic_t panic;
};

typedef enum {
    HEAP_PAIR,
    HEAP_CLOSURE,
    HEAP_ARRAY,
    HEAP_TABLE,
    HEAP_CONTIN,
    HEAP_BIGNUM,
    HEAP_SYM,
    HEAP_TEMPLATE,
} aq_heap_type_t;
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

    AQ_OP_ADDRR,
    AQ_OP_SUBRR,
    AQ_OP_MULRR,
    AQ_OP_DIVRR,
    AQ_OP_CONSRR,

    AQ_OP_ADDRK,
    AQ_OP_SUBRK,
    AQ_OP_MULRK,
    AQ_OP_DIVRK,
    AQ_OP_CONSRK,

    AQ_OP_ADDKK,
    AQ_OP_SUBKK,
    AQ_OP_MULKK,
    AQ_OP_DIVKK,
    AQ_OP_CONSKK,

    AQ_OP_SUBKR,
    AQ_OP_DIVKR,
    AQ_OP_CONSKR,

    AQ_OP_CAR,
    AQ_OP_CDR,

    AQ_OP_LOADK,
} aq_op_t;

#define ENCODE_ABC(op, a, b, c)                                                \
    ((((uint8_t)(op & 0xFF)) << 24) +                                          \
     (((uint8_t)(a & 0xFF) << 16) + (((uint8_t)(b & 0xFF) << 8)) +             \
      ((uint8_t)(c & 0xFF))))
#define ENCODE_AD(op, a, d)                                                    \
    ((((uint8_t)(op & 0xFF)) << 24) +                                          \
     (((uint8_t)(a & 0xFF) << 16) + ((uint8_t)(d & 0xFFFF))))

#endif
