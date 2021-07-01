#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "object.h"
#include "gc.h"

#define GET_OP(inst) ((inst) >> 24)
#define GET_A(inst) (((inst) >> 16) & 0xFF)
#define GET_B(inst) (((inst) >> 8) & 0xFF)
#define GET_C(inst) (inst & 0xFF)
#define GET_D(inst) (inst & 0xFFFF)

#define GET_RA(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_A(inst)))
#define GET_RB(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_B(inst)))
#define GET_RC(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_C(inst)))

#define GET_KA(t, inst) (t->lits[GET_A(inst)])
#define GET_KB(t, inst) (t->lits[GET_B(inst)])
#define GET_KC(t, inst) (t->lits[GET_C(inst)])
#define GET_KD(t, inst) (t->lits[GET_D(inst)])

#define ADDI_OP(a, b) ((a) + (b))
#define SUBI_OP(a, b) ((a) - (b))
#define MULI_OP(a, b) ((a) * (b))
#define DIVI_OP(a, b) ((a) / (b))

#define EQ_OP(a, b)                                                            \
    {                                                                          \
        if (!obj_eq(a, b)) {                                                   \
            insts++;                                                           \
        }                                                                      \
    }

#define LT_OP(a, b) ((a) < (b))
#define LTE_OP(a, b) ((a) <= (b))

#define COMP_OP(aq, v1, v2, inst, op)                                          \
    {                                                                          \
        if (OBJ_IS_NUM(v1) && OBJ_IS_NUM(v2)) {                                \
            double vv1 = OBJ_DECODE_NUM(v1);                                   \
            double vv2 = OBJ_DECODE_NUM(v2);                                   \
            if (!op(vv1, vv2)) {                                               \
                insts++;                                                       \
            }                                                                  \
        } else {                                                               \
            aq_panic(aq, AQ_ERR_INVALID_COMP);                                 \
        }                                                                      \
    }

#define COMPRR(aq, op) COMP_OP(aq, GET_RA(aq, inst), GET_RB(aq, inst), inst, op)
#define COMPKR(aq, op) COMP_OP(aq, GET_KA(t, inst), GET_RB(aq, inst), inst, op)
#define COMPRK(aq, op) COMP_OP(aq, GET_RA(aq, inst), GET_KB(t, inst), inst, op)
#define COMPKK(aq, op) COMP_OP(aq, GET_KA(t, inst), GET_KB(t, inst), inst, op)

#define ARITH_OPS(aq, v1, v2, dest, op)                                        \
    {                                                                          \
        if (OBJ_IS_NUM(v1) && OBJ_IS_NUM(v2)) {                                \
            double vv1 = OBJ_DECODE_NUM(v1);                                   \
            double vv2 = OBJ_DECODE_NUM(v2);                                   \
            OBJ_ENCODE_NUM(dest, op(vv1, vv2));                                \
        } else {                                                               \
            aq_panic(aq, AQ_ERR_INVALID_ARITH);                                \
        }                                                                      \
    }

#define ARITHRR(aq, op)                                                        \
    ARITH_OPS(aq, GET_RB(aq, inst), GET_RC(aq, inst), GET_RA(aq, inst), op)

#define ARITHRK(aq, t, op)                                                     \
    ARITH_OPS(aq, GET_RB(aq, inst), GET_KC(t, inst), GET_RA(aq, inst), op)

#define ARITHKR(aq, t, op)                                                     \
    ARITH_OPS(aq, GET_KB(t, inst), GET_RC(aq, inst), GET_RA(aq, inst), op)

#define ARITHKK(aq, t, op)                                                     \
    ARITH_OPS(aq, GET_KB(t, inst), GET_KC(t, inst), GET_RA(aq, inst), op)

#define CONS(aq, v1, v2)                                                       \
    {                                                                          \
        aq_pair_t *pair = GC_NEW(aq, aq_pair_t, HEAP_PAIR);                    \
        pair->car = v1;                                                        \
        pair->cdr = v2;                                                        \
        OBJ_ENCODE_PAIR(GET_RA(aq, inst), pair);                               \
    }

size_t hash_obj(aq_obj_t obj) {
    switch (obj.t) {
    case AQ_OBJ_CLOSURE:
    case AQ_OBJ_BIGNUM:
    case AQ_OBJ_PAIR:
    case AQ_OBJ_TABLE:
    case AQ_OBJ_ARRAY:
    case AQ_OBJ_CONTIN:
        return CAST(obj.v.h, size_t);
    case AQ_OBJ_SYM: {
        size_t total = 0;
        aq_sym_t *sym = OBJ_DECODE_SYM(obj);
        for (size_t i = 0; i < sym->l; i++) {
            total = (total << 4) + sym->s[i];
            size_t g = total & 0xf0000000;
            if (g != 0) {
                total = total ^ (g >> 24);
                total = total ^ g;
            }
        }
        return total;
    }
    case AQ_OBJ_NUM:
        return CAST(OBJ_DECODE_NUM(obj), size_t);
    case AQ_OBJ_TRUE:
        return 2;
    case AQ_OBJ_FALSE:
        return 1;
    case AQ_OBJ_NIL:
        return 0;
    case AQ_OBJ_CHAR:
        return OBJ_DECODE_CHAR(obj);
    default:
        printf("internal error: unimplemented hash case\n");
        exit(EXIT_FAILURE);
    }
}

bool obj_eq(aq_obj_t ob1, aq_obj_t ob2) {
    if (ob1.t != ob2.t)
        return false;
    switch (ob1.t) {
    case AQ_OBJ_NIL:
    case AQ_OBJ_TRUE:
    case AQ_OBJ_FALSE:
        return true;
    case AQ_OBJ_NUM:
        return ob1.v.n == ob2.v.n;
    case AQ_OBJ_CHAR:
        return ob1.v.c == ob2.v.c;
    case AQ_OBJ_SYM:
    case AQ_OBJ_CLOSURE:
    case AQ_OBJ_PAIR:
    case AQ_OBJ_CONTIN:
    case AQ_OBJ_BIGNUM:
    case AQ_OBJ_ARRAY:
    case AQ_OBJ_TABLE:
        return ob1.v.h == ob2.v.h;
    }
}

aq_obj_t table_search(aq_state_t *aq, aq_obj_t tbl_obj, aq_obj_t key) {
    if (OBJ_IS_TABLE(tbl_obj)) {
        aq_tbl_t *tbl = OBJ_DECODE_HEAP(tbl_obj, aq_tbl_t);
        size_t idx = hash_obj(key) % tbl->buckets_sz;
        for (aq_tbl_entry_t *entry = tbl->buckets[idx]; entry;
             entry = entry->n) {
            if (obj_eq(entry->k, key)) {
                return entry->v;
            }
        }
        aq_obj_t ret;
        OBJ_ENCODE_NIL(ret);
        return ret;
    }
    aq->panic(aq, AQ_ERR_NOT_TABLE);
    return tbl_obj; /* this will never be reached */
}

void table_set(aq_state_t *aq, aq_obj_t tbl_obj, aq_obj_t key, aq_obj_t val) {
    if (OBJ_IS_TABLE(tbl_obj)) {
        aq_tbl_t *tbl = OBJ_DECODE_HEAP(tbl_obj, aq_tbl_t);
        size_t idx = hash_obj(key) % tbl->buckets_sz;
        for (aq_tbl_entry_t *entry = tbl->buckets[idx]; entry;
             entry = entry->n) {
            if (obj_eq(entry->k, key)) {
                entry->v = val;
                return;
            }
        }
        aq_tbl_entry_t *new_entry = aq->alloc(NULL, 0, sizeof(aq_tbl_entry_t));
        new_entry->n = tbl->buckets[idx];
        tbl->buckets[idx] = new_entry;
        new_entry->k = key;
        new_entry->v = val;
        tbl->entries++;
    } else {
        aq->panic(aq, AQ_ERR_NOT_TABLE);
    }
}

aq_obj_t aq_execute_closure(aq_state_t *aq, aq_obj_t obj) {
    aq_closure_t *c = OBJ_DECODE_CLOSURE(obj);
    aq_template_t *t = c->t;

    const uint32_t *insts = t->code;
    uint32_t inst;
    while (1) {
        switch (GET_OP((inst = (*(insts++))))) {
        case AQ_OP_RETR:
            return GET_RA(aq, inst);
        case AQ_OP_RETK:
            return GET_KD(t, inst);
        case AQ_OP_MOVR:
            GET_RA(aq, inst) = GET_RB(aq, inst);
            break;
        case AQ_OP_MOVK:
            GET_RA(aq, inst) = GET_KD(t, inst);
            break;
        case AQ_OP_NIL:
            OBJ_ENCODE_NIL(GET_RA(aq, inst));
            break;

        case AQ_OP_ADDRR:
            ARITHRR(aq, ADDI_OP);
            break;
        case AQ_OP_SUBRR:
            ARITHRR(aq, SUBI_OP);
            break;
        case AQ_OP_MULRR:
            ARITHRR(aq, MULI_OP);
            break;
        case AQ_OP_DIVRR:
            ARITHRR(aq, DIVI_OP);
            break;
        case AQ_OP_CONSRR:
            CONS(aq, GET_RB(aq, inst), GET_RC(aq, inst));
            break;

        case AQ_OP_ADDRK:
            ARITHRK(aq, t, ADDI_OP);
            break;
        case AQ_OP_SUBRK:
            ARITHRK(aq, t, SUBI_OP);
            break;
        case AQ_OP_MULRK:
            ARITHRK(aq, t, MULI_OP);
            break;
        case AQ_OP_DIVRK:
            ARITHRK(aq, t, DIVI_OP);
            break;
        case AQ_OP_CONSRK:
            CONS(aq, GET_RB(aq, inst), GET_KC(t, inst));
            break;

        case AQ_OP_SUBKR:
            ARITHKR(aq, t, SUBI_OP);
            break;
        case AQ_OP_DIVKR:
            ARITHKR(aq, t, DIVI_OP);
            break;
        case AQ_OP_CONSKR:
            CONS(aq, GET_KB(t, inst), GET_RC(aq, inst));
            break;

        case AQ_OP_ADDKK:
            ARITHKK(aq, t, ADDI_OP);
            break;
        case AQ_OP_SUBKK:
            ARITHKK(aq, t, SUBI_OP);
            break;
        case AQ_OP_MULKK:
            ARITHKK(aq, t, MULI_OP);
            break;
        case AQ_OP_DIVKK:
            ARITHKK(aq, t, DIVI_OP);
            break;
        case AQ_OP_CONSKK:
            CONS(aq, GET_KB(t, inst), GET_KC(t, inst));
            break;

        case AQ_OP_CAR:
            if (OBJ_IS_PAIR(GET_RB(aq, inst)))
                GET_RA(aq, inst) = OBJ_GET_CAR(GET_RB(aq, inst));
            else
                aq_panic(aq, AQ_ERR_NOT_PAIR);
            break;
        case AQ_OP_CDR:
            if (OBJ_IS_PAIR(GET_RB(aq, inst)))
                GET_RA(aq, inst) = OBJ_GET_CDR(GET_RB(aq, inst));
            else
                aq_panic(aq, AQ_ERR_NOT_PAIR);
            break;
        case AQ_OP_TABNEW:
            OBJ_ENCODE_TABLE(GET_RA(aq, inst), aq_new_table(aq, GET_D(inst)));
            break;
        case AQ_OP_TABGETR:
            GET_RA(aq, inst) =
                table_search(aq, GET_RB(aq, inst), GET_RC(aq, inst));
            break;
        case AQ_OP_TABGETK:
            GET_RA(aq, inst) =
                table_search(aq, GET_RB(aq, inst), GET_KC(t, inst));
            break;
        case AQ_OP_TABSETRR:
            table_set(aq, GET_RA(aq, inst), GET_RB(aq, inst), GET_RC(aq, inst));
            break;
        case AQ_OP_TABSETKR:
            table_set(aq, GET_RA(aq, inst), GET_KB(t, inst), GET_RC(aq, inst));
            break;
        case AQ_OP_TABSETRK:
            table_set(aq, GET_RA(aq, inst), GET_RB(aq, inst), GET_KC(t, inst));
            break;
        case AQ_OP_TABSETKK:
            table_set(aq, GET_RA(aq, inst), GET_KB(t, inst), GET_KC(t, inst));
            break;
        case AQ_OP_JMP:
            insts += GET_A(inst) ? -1 * GET_D(inst) : GET_D(inst);
            break;

        case AQ_OP_GGETR:
            GET_RA(aq, inst) = table_search(aq, aq->g, GET_RC(aq, inst));
            break;
        case AQ_OP_GGETK:
            GET_RA(aq, inst) = table_search(aq, aq->g, GET_KC(t, inst));
            break;

        case AQ_OP_GSETRR:
            table_set(aq, aq->g, GET_RA(aq, inst), GET_RB(aq, inst));
            break;
        case AQ_OP_GSETKR:
            table_set(aq, aq->g, GET_KA(t, inst), GET_RB(aq, inst));
            break;
        case AQ_OP_GSETRK:
            table_set(aq, aq->g, GET_RA(aq, inst), GET_KB(t, inst));
            break;
        case AQ_OP_GSETKK:
            table_set(aq, aq->g, GET_KA(t, inst), GET_KB(t, inst));
            break;

        case AQ_OP_EQRR:
            EQ_OP(GET_RA(aq, inst), GET_RB(aq, inst));
            break;
        case AQ_OP_EQRK:
            EQ_OP(GET_RA(aq, inst), GET_KB(t, inst));
            break;
        case AQ_OP_EQKR:
            EQ_OP(GET_KA(t, inst), GET_RB(aq, inst));
            break;
        case AQ_OP_EQKK:
            EQ_OP(GET_KA(t, inst), GET_KB(t, inst));
            break;

        case AQ_OP_LTRR:
            COMPRR(aq, LT_OP);
            break;
        case AQ_OP_LTRK:
            COMPRK(aq, LT_OP);
            break;
        case AQ_OP_LTKR:
            COMPKR(aq, LT_OP);
            break;
        case AQ_OP_LTKK:
            COMPKK(aq, LT_OP);
            break;

        case AQ_OP_LTERR:
            COMPRR(aq, LTE_OP);
            break;
        case AQ_OP_LTERK:
            COMPRK(aq, LTE_OP);
            break;
        case AQ_OP_LTEKR:
            COMPKR(aq, LTE_OP);
            break;
        case AQ_OP_LTEKK:
            COMPKK(aq, LTE_OP);
            break;
        }
    }
}

aq_obj_t aq_init_test_closure(aq_state_t *aq) {

    aq_template_t *t = GC_NEW(aq, aq_template_t, HEAP_TEMPLATE);
    t->bit = HEAP_TEMPLATE;
    t->name_sz = 4;

    char *name = aq->alloc(NULL, 0, sizeof(char) * 5);
    memcpy(name, "test", t->name_sz);
    name[4] = 0;
    t->name = name;

    t->lits_sz = 3;
    aq_obj_t *lits = aq->alloc(NULL, 0, t->lits_sz * sizeof(aq_obj_t));
    OBJ_ENCODE_NUM(lits[0], 3.312);
    OBJ_ENCODE_SYM(lits[1], aq_intern_sym(aq, "thing", 5));
    OBJ_ENCODE_NUM(lits[2], 5.0);
    t->lits = lits;

    t->code_sz = 15;
    uint32_t *code = aq->alloc(NULL, 0, t->code_sz * sizeof(uint32_t));
    code[0] = ENCODE_ABC(AQ_OP_GSETKK, 1, 2, 0);
    code[1] = ENCODE_AD(AQ_OP_GGETK, 0, 1);
    code[2] = ENCODE_AD(AQ_OP_RETR, 0, 0);
    t->code = code;

    aq_closure_t *c = GC_NEW(aq, aq_closure_t, HEAP_CLOSURE);
    c->t = t;

    aq_obj_t ret;
    OBJ_ENCODE_CLOSURE(ret, c);
    return ret;
}
