#include "dump.h"

static void dump_inst(uint32_t inst, size_t line, FILE *file) {
    switch (GET_OP(inst)) {
#define OP_USAGE(name, encoding)                                               \
    case AQ_OP_##name:                                                         \
        if (encoding == 0)                                                     \
            fprintf(file, "%ld: %-10s %d %d %d\n", line, #name, GET_A(inst),   \
                    GET_B(inst), GET_C(inst));                                 \
        else                                                                   \
            fprintf(file, "%ld: %-10s %d %d\n", line, #name, GET_A(inst),      \
                    GET_D(inst));                                              \
        break;
#include "ops.h"
#undef OP_USAGE
    }
}

static void dump_lit(aq_obj_t lit, size_t line, FILE *file) {
    if (lit.t == AQ_OBJ_NUM)
        fprintf(file, "%ld: %f\n", line, lit.v.n);
    else {
        aq_sym_t *sym = (aq_sym_t *)lit.v.h;
        fprintf(file, "%ld: %.*s\n", line, (int)sym->l, sym->s);
    }
}

static void dump_template(aq_template_t *t, FILE *file) {
    fprintf(file, "template: %ld literals, %ld instructions\n", t->lits_sz,
            t->code_sz);
    fprintf(file, "\ninstructions:\n");
    for (size_t i = 0; i < t->code_sz; i++) {
        dump_inst(t->code[i], i, file);
    }
    fprintf(file, "\nliterals:\n");
    for (size_t i = 0; i < t->lits_sz; i++) {
        dump_lit(t->lits[i], i, file);
    }
}

void aq_dump_closure(aq_closure_t *c, FILE *file) {
    dump_template(c->t, file);
}
