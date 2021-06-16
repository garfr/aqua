#ifndef AQUA_H

#define AQUA_VERSION "0.1"

typedef struct aq_state_t aq_state_t;

aq_state_t *aq_init_state();
void aq_deinit_state(aq_state_t *aq);

void aq_print_version();

#endif
