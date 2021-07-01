#ifndef STATE_H
#define STATE_H

#include "aqua.h"
#include "object.h"
#include "gc.h"
#include "types.h"

void aq_panic(aq_state_t *aq, aq_err_t err);

aq_sym_t *aq_intern_sym(aq_state_t *aq, const char *str, size_t sz);

void aq_freeze_var(aq_state_t *aq, aq_obj_t *obj);
void aq_unfreeze_var(aq_state_t *aq, aq_obj_t *obj);

#endif

