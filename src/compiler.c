#include "compiler.h"
#include "gc.h"
#include "string.h"
#include "helpers.h"

#define INIT_CODE_CAP 8
#define INIT_LITS_CAP 8

/* the state needed to compile one function, these are created recursively and
 * the final state of it after compilation is used to construct templates and
 * then a simple closure */
typedef struct {
    uint32_t *code;
    size_t code_sz;
    size_t code_cap;

    size_t next_reg;

    aq_obj_t *lits;
    size_t lits_sz;
    size_t lits_cap;
} comp_fn;

/* after the code to generate a value is created, this object is passed to point
 * where the object will be */
typedef struct {
    enum {
        LOC_REG,
        LOC_LIT,
    } t;
    size_t idx;
} val_loc;

typedef enum { ARITH_ADD, ARITH_SUB, ARITH_MUL, ARITH_DIV } arith_op;

void add_inst(aq_state_t *aq, comp_fn *fn, uint32_t inst) {
    if (fn->code_sz + 1 >= fn->code_cap) {
        fn->code = aq->alloc(fn->code, fn->code_cap * sizeof(uint32_t),
                             fn->code_cap * sizeof(uint32_t) * 2);
        fn->code_cap *= 2;
    }
    fn->code[fn->code_sz++] = inst;
}

val_loc add_lit(aq_state_t *aq, comp_fn *fn, aq_obj_t obj) {
    if (fn->lits_sz + 1 >= fn->lits_cap) {
        fn->lits = aq->alloc(fn->lits, fn->lits_cap * sizeof(aq_obj_t),
                             fn->lits_cap * sizeof(aq_obj_t) * 2);
        fn->lits_cap *= 2;
    }
    fn->lits[fn->lits_sz] = obj;

    val_loc ret;
    ret.t = LOC_LIT;
    ret.idx = fn->lits_sz;
    fn->lits_sz++;
    return ret;
}

static const uint8_t arith_op_lookup[4][2][2] = {
    [ARITH_ADD] =
        {
            [LOC_REG] =
                {
                    [LOC_REG] = AQ_OP_ADDRR,
                    [LOC_LIT] = AQ_OP_ADDRK,
                },
            [LOC_LIT] =
                {
                    [LOC_REG] = AQ_OP_ADDKR,
                    [LOC_LIT] = AQ_OP_ADDKK,
                },
        },
    [ARITH_SUB] =
        {
            [LOC_REG] =
                {
                    [LOC_REG] = AQ_OP_SUBRR,
                    [LOC_LIT] = AQ_OP_SUBRK,
                },
            [LOC_LIT] =
                {
                    [LOC_REG] = AQ_OP_SUBKR,
                    [LOC_LIT] = AQ_OP_SUBKK,
                },
        },
    [ARITH_MUL] =
        {
            [LOC_REG] =
                {
                    [LOC_REG] = AQ_OP_MULRR,
                    [LOC_LIT] = AQ_OP_MULRK,
                },
            [LOC_LIT] =
                {
                    [LOC_REG] = AQ_OP_MULKR,
                    [LOC_LIT] = AQ_OP_MULKK,
                },
        },
    [ARITH_DIV] =
        {
            [LOC_REG] =
                {
                    [LOC_REG] = AQ_OP_DIVRR,
                    [LOC_LIT] = AQ_OP_DIVRK,
                },
            [LOC_LIT] =
                {
                    [LOC_REG] = AQ_OP_DIVKR,
                    [LOC_LIT] = AQ_OP_DIVKK,
                },
        },
};

val_loc generate_form(aq_state_t *aq, aq_obj_t form, comp_fn *fn);

val_loc generate_arith(aq_state_t *aq, arith_op op, aq_obj_t args,
                       comp_fn *fn) {
    if (args.t != AQ_OBJ_PAIR && OBJ_GET_CDR(args).t != AQ_OBJ_PAIR) {
        printf("invalid arguments to arithmetic op\n");
        exit(EXIT_FAILURE);
    }
    val_loc loc1 = generate_form(aq, OBJ_GET_CAR(args), fn);
    val_loc loc2 = generate_form(aq, OBJ_GET_CAR(OBJ_GET_CDR(args)), fn);

    val_loc ret = {LOC_REG, fn->next_reg++};

    add_inst(aq, fn,
             ENCODE_ABC(arith_op_lookup[op][loc1.t][loc2.t], ret.idx, loc1.idx,
                        loc2.idx));
    return ret;
}

val_loc generate_funcall(aq_state_t *aq, aq_obj_t head, aq_obj_t args,
                         comp_fn *fn) {
    if (head.t == AQ_OBJ_SYM) {
        aq_sym_t *sym = OBJ_DECODE_SYM(head);
        if (streq(sym->s, "+", sym->l, 1))
            return generate_arith(aq, ARITH_ADD, args, fn);
        if (streq(sym->s, "-", sym->l, 1))
            return generate_arith(aq, ARITH_SUB, args, fn);
        if (streq(sym->s, "*", sym->l, 1))
            return generate_arith(aq, ARITH_MUL, args, fn);
        if (streq(sym->s, "/", sym->l, 1))
            return generate_arith(aq, ARITH_DIV, args, fn);
    }
    printf("cannot compile real functions yet\n");
    exit(EXIT_FAILURE);
}

val_loc generate_form(aq_state_t *aq, aq_obj_t form, comp_fn *fn) {
    switch (form.t) {
    case AQ_OBJ_NUM:
        return add_lit(aq, fn, form);
    case AQ_OBJ_PAIR:
        return generate_funcall(aq, OBJ_GET_CAR(form), OBJ_GET_CDR(form), fn);
    default:
        printf("cannot compile form %d\n", form.t);
        exit(EXIT_FAILURE);
    }
    (void)form;
    aq_obj_t lit;
    OBJ_ENCODE_NUM(lit, 3.14159);
    return add_lit(aq, fn, lit);
}

comp_fn init_fn(aq_state_t *aq) {
    comp_fn ret;

    ret.code_sz = 0;
    ret.code_cap = INIT_CODE_CAP;
    ret.code = aq->alloc(NULL, 0, sizeof(uint32_t) * ret.code_cap);

    ret.lits_sz = 0;
    ret.lits_cap = INIT_LITS_CAP;
    ret.lits = aq->alloc(NULL, 0, sizeof(aq_obj_t) * ret.lits_cap);

    ret.next_reg = 0;

    return ret;
}

void deinit_fn(aq_state_t *aq, comp_fn *fn) {
    aq->alloc(fn->code, 0, 0);
    aq->alloc(fn->lits, 0, 0);
    fn->code = NULL;
    fn->lits = NULL;
}

aq_closure_t *compile_form(aq_state_t *aq, aq_obj_t form) {
    comp_fn global_fn = init_fn(aq);

    val_loc val = generate_form(aq, form, &global_fn);
    add_inst(aq, &global_fn,
             ENCODE_AD(val.t == LOC_LIT ? AQ_OP_RETK : AQ_OP_RETR, 0, val.idx));

    aq_template_t *t = GC_NEW(aq, aq_template_t, HEAP_TEMPLATE);

    uint32_t *code = aq->alloc(NULL, 0, sizeof(uint32_t) * global_fn.code_sz);
    memcpy(code, global_fn.code, sizeof(uint32_t) * global_fn.code_sz);
    t->code_sz = global_fn.code_sz;
    t->code = code;

    aq_obj_t *lits = aq->alloc(NULL, 0, sizeof(aq_obj_t) * global_fn.lits_sz);
    memcpy(lits, global_fn.lits, sizeof(aq_obj_t) * global_fn.lits_sz);
    t->lits_sz = global_fn.lits_sz;
    t->lits = lits;

    aq_closure_t *c = GC_NEW(aq, aq_closure_t, HEAP_CLOSURE);
    c->t = t;

    deinit_fn(aq, &global_fn);
    return c;
}
