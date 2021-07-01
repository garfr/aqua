#ifndef TYPES_H
#define TYPES_H

#include "aqua.h"

/* items to be included in all heap objects */
#define HEAP_OBJ_HEADER                                                        \
    struct aq_heap_obj_t *gc_forward;                                          \
    uint8_t bit;                                                               \
    uint8_t mark

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

typedef struct aq_tbl_entry_t {
    struct aq_tbl_entry_t *n;
    aq_obj_t k;
    aq_obj_t v;
} aq_tbl_entry_t;

typedef struct {
    HEAP_OBJ_HEADER;
    size_t entries;
    size_t buckets_sz;
    aq_tbl_entry_t **buckets;
} aq_tbl_t;

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

    aq_sym_tbl_t syms;

    aq_panic_t panic;

    aq_heap_obj_t *gc_root;
    aq_obj_t g;

    /* variables frozen for the GC */
    size_t frozen_sz;
    aq_obj_t **frozen;
    size_t frozen_cap;
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
struct aq_heap_obj_t {
    HEAP_OBJ_HEADER;
};

typedef struct {
    HEAP_OBJ_HEADER;
    aq_obj_t car;
    aq_obj_t cdr;
} aq_pair_t;

typedef enum {
    AQ_OP_RETR,
    AQ_OP_RETK,

    AQ_OP_MOVR,
    AQ_OP_MOVK,

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

    AQ_OP_TABNEW,

    AQ_OP_TABSETRR,
    AQ_OP_TABSETKR,
    AQ_OP_TABSETRK,
    AQ_OP_TABSETKK,

    AQ_OP_TABGETR,
    AQ_OP_TABGETK,

    AQ_OP_JMP,

    AQ_OP_EQRR,
    AQ_OP_EQKR,
    AQ_OP_EQRK,
    AQ_OP_EQKK,

    AQ_OP_LTRR,
    AQ_OP_LTKR,
    AQ_OP_LTRK,
    AQ_OP_LTKK,

    AQ_OP_LTERR,
    AQ_OP_LTEKR,
    AQ_OP_LTERK,
    AQ_OP_LTEKK,

    AQ_OP_GGETR,
    AQ_OP_GGETK,

    AQ_OP_GSETRR,
    AQ_OP_GSETRK,
    AQ_OP_GSETKR,
    AQ_OP_GSETKK,
} aq_op_t;

#define ENCODE_ABC(op, a, b, c)                                                \
    ((((uint8_t)(op & 0xFF)) << 24) +                                          \
     (((uint8_t)(a & 0xFF) << 16) + (((uint8_t)(b & 0xFF) << 8)) +             \
      ((uint8_t)(c & 0xFF))))
#define ENCODE_AD(op, a, d)                                                    \
    ((((uint8_t)(op & 0xFF)) << 24) +                                          \
     (((uint8_t)(a & 0xFF) << 16) + ((uint8_t)(d & 0xFFFF))))

#endif
