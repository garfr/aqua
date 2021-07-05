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

    aq_var2(aq, file, closure);

    file = aq_read_file(aq, argv[1]);

    if (dump_bc) {
        aq_obj_t closure = aq_compile_form(aq, file);
        aq_print_closure(aq, closure, stdout);
    }

    aq_eval(aq, file);

    aq_release2(aq, file, closure);

    aq_deinit_state(aq);
    return EXIT_SUCCESS;
}
