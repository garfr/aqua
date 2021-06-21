#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "object.h"
#include "gc.h"

#define GET_OP(inst) ((inst) >> 24)
#define GET_A(inst) (((inst) >> 16) & 0xFF)
#define GET_B(inst) (((inst) >> 8) & 0xFF)
#define GET_C(inst) (inst & 0xFF)

aq_obj_t aq_execute_closure(aq_state_t *aq, aq_obj_t obj) {
    aq_closure_t *c = (aq_closure_t *)GET_HEAP_PTR(obj);
    aq_template_t *t = c->t;

    /* test */
    aq->vars[0] = aq_encode_int(20);

    const uint32_t *insts = t->code;
    uint32_t inst;
    while (1) {
        switch (GET_OP((inst = (*(insts++))))) {
        case AQ_OP_RET:
            return *(aq->vars + aq->cur_call->var_pos + GET_A(inst));
        case AQ_OP_MOV:
            *(aq->vars + aq->cur_call->var_pos + GET_B(inst)) =
                *(aq->vars + aq->cur_call->var_pos + GET_A(inst));
        }
    }
}

aq_obj_t aq_init_test_closure(aq_state_t *aq) {
    aq_template_t *t = aq_gc_alloc(&aq->heap, sizeof(aq_template_t));
    t->name_sz = 4;
    char *name = aq_gc_alloc(&aq->heap, sizeof(char) * t->name_sz);
    memcpy(name, "test", t->name_sz);
    t->name = name;

    t->code_sz = 2;
    uint32_t *code = aq_gc_alloc(&aq->heap, sizeof(uint32_t) * t->code_sz);
    code[0] = ENCODE_ABC(AQ_OP_MOV, 1, 0, 0);
    code[1] = ENCODE_ABC(AQ_OP_RET, 1, 0, 0);
    t->code = code;

    aq_closure_t *c = aq_gc_alloc(&aq->heap, sizeof(aq_closure_t));
    c->t = t;

    return aq_encode_closure(c);
}
