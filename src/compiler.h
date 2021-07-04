#ifndef COMPILER_H
#define COMPILER_H

#include "types.h"

aq_closure_t *compile_form(aq_state_t *aq, aq_obj_t form);

#endif
