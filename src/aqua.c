#include <stdio.h>

#include "aqua.h"
#include "state.h"

void aq_print_version() {
    puts(AQUA_VERSION);
}

size_t aq_get_mem_used(aq_state_t *aq) {
    return aq->heap.top - aq->heap.mem;
}
