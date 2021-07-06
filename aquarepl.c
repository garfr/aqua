#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>

#include "aqua.h"

static const char *err_names[] = {
    [AQ_ERR_OOM] = "Out of Memory",
    [AQ_ERR_INVALID_ARITH] = "Invalid Arithmetic (Type Error)",
    [AQ_ERR_NOT_PAIR] = "Not Pair (Type Error)",
    [AQ_ERR_NOT_TABLE] = "Not Table (Type Error)",
    [AQ_ERR_INVALID_FILENAME] = "Invalid Filename",
};

static int error_handler(aq_state_t *aq, aq_err_t err) {
    (void)aq;
    printf("Err: %s\n", err_names[err]);
    return 1;
}

static void *libc_alloc(void *ptr, size_t old_sz, size_t new_sz) {
    if (new_sz == 0) {
        free(ptr);
        return NULL;
    } else if (old_sz == 0) {
        return calloc(1, new_sz);
    } else {
        return realloc(ptr, new_sz);
    }
}

int main() {
    aq_state_t *aq = aq_init_state(libc_alloc);
    aq_set_panic(aq, error_handler);

    aq_var2(aq, form, res);

    aq_eval_file(aq, "test.scm");

    res = aq_eval(aq, form);

    aq_collect_garbage(aq);

    aq_release2(aq, form, res);

    aq_collect_garbage(aq);

    aq_deinit_state(aq);
    return 0;
}
