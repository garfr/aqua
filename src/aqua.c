#include <stdio.h>

#include "aqua.h"
#include "dump.h"
#include "vm.h"
#include "compiler.h"
#include "state.h"

void aq_print_version() {
    puts(AQUA_VERSION);
}

aq_obj_t aq_eval(aq_state_t *aq, aq_obj_t obj) {
    aq_closure_t *c = compile_form(aq, obj);
    return aq_execute_closure(aq, c);
}

aq_obj_t aq_compile_form(aq_state_t *aq, aq_obj_t obj) {
    aq_closure_t *c = compile_form(aq, obj);
    aq_obj_t ret;
    OBJ_ENCODE_CLOSURE(ret, c);
    return ret;
}

void aq_print_closure(aq_state_t *aq, aq_obj_t obj, FILE *file) {
    (void)aq;
    aq_closure_t *c = OBJ_DECODE_HEAP(obj, aq_closure_t);
    aq_dump_closure(c, file);
}

static void print_obj_inner(aq_obj_t obj, FILE *file) {
    aq_obj_type_t typ = aq_get_type(obj);
    switch (typ) {
    case AQ_OBJ_NUM:
        fprintf(file, "%f", aq_get_num(obj));
        return;
    case AQ_OBJ_TRUE:
        fprintf(file, "#t");
        return;
    case AQ_OBJ_FALSE:
        fprintf(file, "#f");
        return;
    case AQ_OBJ_CHAR:
        fprintf(file, "'%c'", aq_get_char(obj));
        return;
    case AQ_OBJ_NIL:
        fprintf(file, "nil");
        return;
    case AQ_OBJ_SYM: {
        size_t sz;
        const char *sym = aq_get_sym(obj, &sz);
        fprintf(file, "%.*s", (int)sz, sym);
        break;
    }
    case AQ_OBJ_PAIR: {
        fprintf(file, "(");
        while (obj.t != AQ_OBJ_NIL) {
            print_obj_inner(OBJ_GET_CAR(obj), file);
            obj = OBJ_GET_CDR(obj);
            fprintf(file, " ");
        }
        fprintf(file, ")");
        break;
    }

    case AQ_OBJ_TABLE:
        fprintf(file, "<table>");
        break;
    default:
        fprintf(file, "<invalid type> %d", typ);
        break;
    }
}

void aq_display(aq_state_t *aq, aq_obj_t obj, FILE *file) {
    (void)aq;
    print_obj_inner(obj, file);
    printf("\n");
}
