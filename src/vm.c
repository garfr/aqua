#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dump.h"
#include "vm.h"
#include "object.h"
#include "gc.h"

#define GET_RA(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_A(inst)))
#define GET_RB(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_B(inst)))
#define GET_RC(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_C(inst)))
#define GET_RD(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_D(inst)))

#define GET_KA(t, inst) (t->lits[GET_A(inst)])
#define GET_KB(t, inst) (t->lits[GET_B(inst)])
#define GET_KC(t, inst) (t->lits[GET_C(inst)])
#define GET_KD(t, inst) (t->lits[GET_D(inst)])

#define ADDI_OP(a, b) ((a) + (b))
#define SUBI_OP(a, b) ((a) - (b))
#define MULI_OP(a, b) ((a) * (b))
#define DIVI_OP(a, b) ((a) / (b))

#define EQ_OP(dest, a, b)                                                      \
    {                                                                          \
        if (obj_eq(a, b))                                                      \
            OBJ_ENCODE_TRUE(dest);                                             \
        else                                                                   \
            OBJ_ENCODE_FALSE(dest);                                            \
    }

#define LT_OP(a, b) ((a) < (b))
#define LTE_OP(a, b) ((a) <= (b))

#define COMP_OP(aq, dest, v1, v2, inst, op)                                    \
    {                                                                          \
        if (OBJ_IS_NUM(v1) && OBJ_IS_NUM(v2)) {                                \
            double vv1 = OBJ_DECODE_NUM(v1);                                   \
            double vv2 = OBJ_DECODE_NUM(v2);                                   \
            if (op(vv1, vv2))                                                  \
                OBJ_ENCODE_TRUE(dest);                                         \
            else                                                               \
                OBJ_ENCODE_FALSE(dest);                                        \
        } else {                                                               \
            aq_panic(aq, AQ_ERR_INVALID_COMP);                                 \
        }                                                                      \
    }

#define COMPRR(aq, op)                                                         \
    COMP_OP(aq, GET_RA(aq, inst), GET_RB(aq, inst), GET_RC(aq, inst), inst, op)
#define COMPKR(aq, op)                                                         \
    COMP_OP(aq, GET_RA(aq, inst), GET_KB(t, inst), GET_RC(aq, inst), inst, op)
#define COMPRK(aq, op)                                                         \
    COMP_OP(aq, GET_RA(aq, inst), GET_RB(aq, inst), GET_KC(t, inst), inst, op)
#define COMPKK(aq, op)                                                         \
    COMP_OP(aq, GET_RA(aq, inst), GET_KB(t, inst), GET_KC(t, inst), inst, op)

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

aq_obj_t aq_execute_closure(aq_state_t *aq, aq_closure_t *c) {
    aq_template_t *t = c->t;

    const uint32_t *insts = t->code;
    uint32_t inst;
    while (1) {
        switch (GET_OP((inst = (*(insts++))))) {
        case AQ_OP_RETR:
            return GET_RD(aq, inst);
        case AQ_OP_RETK:
            return GET_KD(t, inst);
        case AQ_OP_RETNIL: {
            aq_obj_t ret;
            OBJ_ENCODE_NIL(ret);
            return ret;
        }
        case AQ_OP_MOVR:
            GET_RA(aq, inst) = GET_RD(aq, inst);
            break;
        case AQ_OP_MOVK:
            GET_RA(aq, inst) = GET_KD(t, inst);
            break;

        case AQ_OP_NIL:
            OBJ_ENCODE_NIL(GET_RD(aq, inst));
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

        case AQ_OP_ADDKR:
            ARITHKR(aq, t, ADDI_OP);
            break;
        case AQ_OP_SUBKR:
            ARITHKR(aq, t, SUBI_OP);
            break;
        case AQ_OP_MULKR:
            ARITHKR(aq, t, MULI_OP);
            break;
        case AQ_OP_DIVKR:
            ARITHKR(aq, t, DIVI_OP);
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

        case AQ_OP_CONSRR:
            CONS(aq, GET_RB(aq, inst), GET_RC(aq, inst));
            break;
        case AQ_OP_CONSRK:
            CONS(aq, GET_RB(aq, inst), GET_KC(t, inst));
            break;
        case AQ_OP_CONSKR:
            CONS(aq, GET_KB(t, inst), GET_RC(aq, inst));
            break;
        case AQ_OP_CONSKK:
            CONS(aq, GET_KB(t, inst), GET_KC(t, inst));
            break;

        case AQ_OP_CAR:
            if (OBJ_IS_PAIR(GET_RD(aq, inst)))
                GET_RA(aq, inst) = OBJ_GET_CAR(GET_RD(aq, inst));
            else
                aq_panic(aq, AQ_ERR_NOT_PAIR);
            break;
        case AQ_OP_CDR:
            if (OBJ_IS_PAIR(GET_RD(aq, inst)))
                GET_RA(aq, inst) = OBJ_GET_CDR(GET_RD(aq, inst));
            else
                aq_panic(aq, AQ_ERR_NOT_PAIR);
            break;

        case AQ_OP_TABNEW:
            OBJ_ENCODE_TABLE(GET_RA(aq, inst), aq_new_table(aq, GET_D(inst)));
            break;

        case AQ_OP_TABSETRR:
            aq_table_set(aq, GET_RA(aq, inst), GET_RB(aq, inst),
                         GET_RC(aq, inst));
            break;
        case AQ_OP_TABSETKR:
            aq_table_set(aq, GET_RA(aq, inst), GET_KB(t, inst),
                         GET_RC(aq, inst));
            break;
        case AQ_OP_TABSETRK:
            aq_table_set(aq, GET_RA(aq, inst), GET_RB(aq, inst),
                         GET_KC(t, inst));
            break;
        case AQ_OP_TABSETKK:
            aq_table_set(aq, GET_RA(aq, inst), GET_KB(t, inst),
                         GET_KC(t, inst));
            break;

        case AQ_OP_TABGETR:
            GET_RA(aq, inst) =
                aq_table_get(aq, GET_RB(aq, inst), GET_RC(aq, inst));
            break;
        case AQ_OP_TABGETK:
            GET_RA(aq, inst) =
                aq_table_get(aq, GET_RB(aq, inst), GET_KC(t, inst));
            break;

        case AQ_OP_JMPF:
            if (OBJ_IS_TRUTHY(GET_RA(aq, inst)))
                insts += GET_D(inst);
            break;
        case AQ_OP_JMPB:
            if (OBJ_IS_TRUTHY(GET_RA(aq, inst)))
                insts -= GET_D(inst);
            break;

        case AQ_OP_GGETR:
            GET_RA(aq, inst) = aq_table_get(aq, aq->g, GET_RD(aq, inst));
            break;
        case AQ_OP_GGETK:
            GET_RA(aq, inst) = aq_table_get(aq, aq->g, GET_KD(t, inst));
            break;

        case AQ_OP_GSETRR:
            aq_table_set(aq, aq->g, GET_RA(aq, inst), GET_RD(aq, inst));
            break;
        case AQ_OP_GSETKR:
            aq_table_set(aq, aq->g, GET_KA(t, inst), GET_RD(aq, inst));
            break;
        case AQ_OP_GSETRK:
            aq_table_set(aq, aq->g, GET_RA(aq, inst), GET_KD(t, inst));
            break;
        case AQ_OP_GSETKK:
            aq_table_set(aq, aq->g, GET_KA(t, inst), GET_KD(t, inst));
            break;

        case AQ_OP_EQRR:
            EQ_OP(GET_RA(aq, inst), GET_RB(aq, inst), GET_RC(aq, inst));
            break;
        case AQ_OP_EQRK:
            EQ_OP(GET_RA(aq, inst), GET_RB(aq, inst), GET_KC(t, inst));
            break;
        case AQ_OP_EQKK:
            EQ_OP(GET_RA(aq, inst), GET_KB(t, inst), GET_KC(t, inst));
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

        case AQ_OP_DISPLAYR:
            aq_display(aq, GET_RD(aq, inst), stdout);
            break;
        case AQ_OP_DISPLAYK:
            aq_display(aq, GET_KD(t, inst), stdout);
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

    t->code_sz = 3;
    uint32_t *code = aq->alloc(NULL, 0, t->code_sz * sizeof(uint32_t));
    code[0] = ENCODE_AD(AQ_OP_GSETKK, 1, 0);
    code[1] = ENCODE_AD(AQ_OP_GGETK, 0, 1);
    code[2] = ENCODE_AD(AQ_OP_RETK, 0, 1);
    t->code = code;

    aq_closure_t *c = GC_NEW(aq, aq_closure_t, HEAP_CLOSURE);
    c->t = t;

    aq_dump_closure(c, stdout);

    aq_obj_t ret;
    OBJ_ENCODE_CLOSURE(ret, c);
    return ret;
}
