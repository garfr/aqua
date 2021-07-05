#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aqua.h"

#define streq(str1, str2) (strcmp(str1, str2) == 0)

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

static const char *err_names[] = {
    [AQ_ERR_OOM] = "Out of Memory",
    [AQ_ERR_INVALID_ARITH] = "Invalid Arithmetic (Type Error)",
    [AQ_ERR_NOT_PAIR] = "Not Pair (Type Error)",
    [AQ_ERR_NOT_TABLE] = "Not Table (Type Error)",
    [AQ_ERR_INVALID_FILENAME] = "Invalid Filename",
    [AQ_ERR_SYNTAX] = "Syntax Error",
};

static int error_handler(aq_state_t *aq, aq_err_t err) {
    (void)aq;
    printf("Err: %s\n", err_names[err]);

    if (err == AQ_ERR_SYNTAX)
        fprintf(stderr, "error: %s\n", aq_get_err_msg(aq));
    return 1;
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        printf("format: ./aquaexec (filename) [flags]\n");
        exit(EXIT_FAILURE);
    }

    bool dump_bc = false;

    for (int i = 2; i < argc; i++) {
        if (streq("-b", argv[i])) {
            dump_bc = true;
        }
    }

    aq_state_t *aq = aq_init_state(libc_alloc);
    aq_set_panic(aq, error_handler);

    aq_var2(aq, file, closure);

    const char *str = aq_read_file(aq, argv[1], &file);
    if (str != NULL) {
        fprintf(stderr, "%s\n", str);
        return EXIT_FAILURE;
    }

    aq_display(aq, file, stdout);

    if (dump_bc) {
        aq_obj_t closure = aq_compile_form(aq, file);
        aq_print_closure(aq, closure, stdout);
    }

    aq_eval(aq, file);

    aq_release2(aq, file, closure);

    aq_deinit_state(aq);
    return EXIT_SUCCESS;
}
