#include <stdlib.h>

#include "state.h"
#include "object.h"

/* found in "aqua.h" */
aq_state_t *aq_init_state(aq_alloc_t alloc) {
    aq_state_t *aq = alloc(NULL, 0, sizeof(aq_state_t));
    aq->alloc = alloc;
    return aq;
}

void aq_deinit_state(aq_state_t *aq) {
    aq->alloc(aq, sizeof(aq_state_t), 0);
}
