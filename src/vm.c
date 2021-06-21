#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "object.h"
#include "gc.h"

#define GET_OP(inst) ((inst) >> 24)
#define GET_A(inst) (((inst) >> 16) & 0xFF)
#define GET_B(inst) (((inst) >> 8) & 0xFF)
#define GET_C(inst) (inst & 0xFF)

#define GET_RA(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_A(inst)))
#define GET_RB(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_B(inst)))
#define GET_RC(aq, inst) (*(aq->vars + aq->cur_call->var_pos + GET_C(inst)))

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

#define ARITH(aq, op)                                                          \
    (ARITH_OPS(aq, GET_RB(aq, inst), GET_RC(aq, inst), GET_RA(aq, inst), op))

aq_obj_t aq_execute_closure(aq_state_t *aq, aq_obj_t obj) {
    aq_closure_t *c = OBJ_DECODE_CLOSURE(obj);
    aq_template_t *t = c->t;

    /* test */
    aq->vars[0] = OBJ_ENCODE_INT(2);
    aq->vars[1] = OBJ_ENCODE_INT(3);
    aq->vars[2] = OBJ_ENCODE_INT(4);

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
        case AQ_OP_ADD:
            ARITH(aq, ADDI_OP);
            break;
        case AQ_OP_SUB:
            ARITH(aq, SUBI_OP);
            break;
        case AQ_OP_MUL:
            ARITH(aq, MULI_OP);
            break;
        case AQ_OP_DIV:
            ARITH(aq, DIVI_OP);
            break;
        }
    }
}

aq_obj_t aq_init_test_closure(aq_state_t *aq) {
    aq_template_t *t = aq_gc_alloc(aq, sizeof(aq_template_t));
    t->name_sz = 4;
    char *name = aq_gc_alloc(aq, sizeof(char) * t->name_sz);
    memcpy(name, "test", t->name_sz);
    t->name = name;

    t->code_sz = 2;
    uint32_t *code = aq_gc_alloc(aq, sizeof(uint32_t) * t->code_sz);
    code[0] = ENCODE_ABC(AQ_OP_ADD, 2, 0, 1);
    code[1] = ENCODE_ABC(AQ_OP_RET, 2, 0, 0);
    t->code = code;

    aq_closure_t *c = aq_gc_alloc(aq, sizeof(aq_closure_t));
    c->t = t;

    return aq_encode_closure(c);
}
