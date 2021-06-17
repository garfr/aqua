#include <stdlib.h>

#include "aqua.h"

void *libc_alloc(void *ptr, size_t old_sz, size_t new_sz) {
    if (new_sz == 0) {
        free(ptr);
        return NULL;
    } else if (old_sz == 0) {
        return malloc(new_sz);
    } else {
        return realloc(ptr, new_sz);
    }
}

int main() {
    aq_state_t *aq = aq_init_state(libc_alloc);
    aq_print_version();
    aq_deinit_state(aq);
    return 0;
}
