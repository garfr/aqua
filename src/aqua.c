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
    aq_dump_closure(c, stdout);
    return aq_execute_closure(aq, c);
}
