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

#define GET_KA(t, inst) (t->nums[GET_A(inst)])
#define GET_KB(t, inst) (t->nums[GET_B(inst)])
#define GET_KC(t, inst) (t->nums[GET_C(inst)])
#define GET_KD(t, inst) (t->nums[GET_D(inst)])

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
        case AQ_OP_LOADI:
            GET_RA(aq, inst) = GET_KD(t, inst);
            break;
        }
    }
}

aq_obj_t aq_init_test_closure(aq_state_t *aq) {
    aq_template_t *t = GC_NEW(aq, aq_template_t);
    t->name_sz = 4;

    char *name = GC_NEW_ARRAY(aq, char, t->name_sz);
    memcpy(name, "test", t->name_sz);
    t->name = name;

    t->nums_sz = 3;
    int64_t *nums = GC_NEW_ARRAY(aq, int64_t, t->nums_sz);
    nums[0] = OBJ_ENCODE_INT(3);
    nums[1] = OBJ_ENCODE_INT(4);
    nums[2] = OBJ_ENCODE_INT(5);
    t->nums = nums;

    t->code_sz = 4;
    uint32_t *code = GC_NEW_ARRAY(aq, uint32_t, t->code_sz);
    code[0] = ENCODE_AD(AQ_OP_LOADI, 0, 0);
    code[1] = ENCODE_ABC(AQ_OP_ADDKK, 1, 0, 1);
    code[2] = ENCODE_ABC(AQ_OP_MULRK, 1, 1, 2);
    code[3] = ENCODE_ABC(AQ_OP_RET, 1, 0, 0);
    t->code = code;

    aq_closure_t *c = GC_NEW(aq, aq_closure_t);
    c->t = t;

    return aq_encode_closure(c);
}
