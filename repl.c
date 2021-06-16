#include "aqua.h"

int main() {
    aq_state_t *aq = aq_init_state();
    aq_print_version();
    aq_deinit_state(aq);
}
