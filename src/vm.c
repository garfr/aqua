#include <string.h>
#include <stdio.h>

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

#define ARITH_OPS(aq, v1, v2, dest, op)                                        \
    {                                                                          \
        if (OBJ_IS_INT(v1) && OBJ_IS_INT(v2)) {                                \
            uint64_t vv1 = OBJ_DECODE_INT(v1);                                 \
            uint64_t vv2 = OBJ_DECODE_INT(v2);                                 \
            dest = OBJ_ENCODE_INT(op(vv1, vv2));                               \
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
        aq_pair_t *pair = GC_NEW(aq, aq_pair_t);                               \
        pair->tt = HEAP_PAIR;                                                  \
        pair->car = v1;                                                        \
        pair->cdr = v2;                                                        \
        GET_RA(aq, inst) = OBJ_ENCODE_PAIR(pair);                              \
    }

aq_obj_t aq_execute_closure(aq_state_t *aq, aq_obj_t obj) {
    aq_closure_t *c = OBJ_DECODE_CLOSURE(obj);
    aq_template_t *t = c->t;

    const uint32_t *insts = t->code;
    uint32_t inst;
    while (1) {
        switch (GET_OP((inst = (*(insts++))))) {
        case AQ_OP_RET:
            return GET_RA(aq, inst);
        case AQ_OP_MOV:
            GET_RA(aq, inst) = GET_RB(aq, inst);
            break;
        case AQ_OP_NIL:
            GET_RA(aq, inst) = OBJ_NIL_VAL;
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
        case AQ_OP_LOADK:
            GET_RA(aq, inst) = GET_KD(t, inst);
            break;
        }
    }
}

aq_obj_t aq_init_test_closure(aq_state_t *aq) {
    uint8_t *t_buf = GC_NEW_BYTES(
        aq,
        sizeof(aq_template_t) + (sizeof(aq_obj_t) * 2) + (sizeof(uint32_t) * 4),
        uint8_t);
    aq_template_t *t = CAST(t_buf, aq_template_t *);
    t->tt = HEAP_TEMPLATE;
    t->name_sz = 4;

    char *name = CAST(t_buf + sizeof(aq_template_t), char *);
    memcpy(name, "test", t->name_sz);
    t->name = name;

    t->lits_sz = 2;
    aq_obj_t *lits =
        CAST(t_buf + sizeof(aq_template_t) + (sizeof(char) * t->name_sz),
             aq_obj_t *);
    lits[0] = OBJ_ENCODE_INT(3);
    lits[1] = OBJ_ENCODE_SYM(aq_intern_sym(aq, "thing", 5));
    t->lits = lits;

    t->code_sz = 4;
    uint32_t *code =
        CAST(t_buf + sizeof(aq_template_t) + (sizeof(char) * t->name_sz) +
                 (sizeof(aq_obj_t) * t->lits_sz),
             uint32_t *);
    code[0] = ENCODE_ABC(AQ_OP_CONSKK, 1, 0, 1);
    code[1] = ENCODE_ABC(AQ_OP_RET, 1, 0, 0);
    t->code = code;

    aq_closure_t *c = GC_NEW(aq, aq_closure_t);
    c->t = t;

    return aq_encode_closure(c);
}
