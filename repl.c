#include <stdlib.h>
#include <stdio.h>

#include "aqua.h"

static void *libc_alloc(void *ptr, size_t old_sz, size_t new_sz) {
    if (new_sz == 0) {
        free(ptr);
        return NULL;
    } else if (old_sz == 0) {
        return malloc(new_sz);
    } else {
        return realloc(ptr, new_sz);
    }
}

static void print_obj(aq_obj_t obj) {
    aq_obj_type_t typ = aq_get_type(obj);
    switch (typ) {
    case AQ_OBJ_INT:
        printf("%ld\n", aq_decode_int(obj));
        return;
    case AQ_OBJ_BOOL:
        printf("#%c\n", aq_decode_bool(obj) ? 't' : 'f');
        return;
    case AQ_OBJ_CHAR:
        printf("%c\n", aq_decode_char(obj));
        return;
    case AQ_OBJ_NIL:
        printf("nil\n");
        return;
    }
    printf("<invalid type>\n");
}

int main() {
    aq_state_t *aq = aq_init_state(libc_alloc);
    aq_print_version();

    aq_obj_t c = aq_encode_char('a');
    aq_obj_t i = aq_encode_int(32);
    aq_obj_t b = aq_encode_bool(false);
    aq_obj_t n = aq_encode_nil();
    print_obj(c);
    print_obj(i);
    print_obj(b);
    print_obj(n);

    aq_deinit_state(aq);
    return 0;
}
