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
#define OP_USAGE(name, _) AQ_OP_##name,
#include "ops.h"
#undef OP_USAGE
    AQ_OP_BURNER /* to avoid comma at the end of enum list */
} aq_op_t;

#define ENCODE_ABC(op, a, b, c)                                                \
    ((((uint8_t)(op & 0xFF)) << 24) +                                          \
     (((uint8_t)(a & 0xFF) << 16) + (((uint8_t)(b & 0xFF) << 8)) +             \
      ((uint8_t)(c & 0xFF))))
#define ENCODE_AD(op, a, d)                                                    \
    ((((uint8_t)(op & 0xFF)) << 24) +                                          \
     (((uint8_t)(a & 0xFF) << 16) + ((uint8_t)(d & 0xFFFF))))

#define GET_OP(inst) ((inst) >> 24)
#define GET_A(inst) (((inst) >> 16) & 0xFF)
#define GET_B(inst) (((inst) >> 8) & 0xFF)
#define GET_C(inst) (inst & 0xFF)
#define GET_D(inst) (inst & 0xFFFF)

#endif
