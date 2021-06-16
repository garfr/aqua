#include <stdlib.h>

#include "state.h"
#include "object.h"

/* found in "aqua.h" */
aq_state_t *aq_init_state() {
    aq_state_t *state = calloc(1, sizeof(aq_state_t));
    return state;
}

void aq_deinit_state(aq_state_t *aq) {
    free(aq);
}
