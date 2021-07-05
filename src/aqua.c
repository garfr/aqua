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
