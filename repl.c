#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "aqua.h"

jmp_buf escape;

static int error_handler(aq_state_t *aq) {
    (void)aq;
    longjmp(escape, 1);
    return 1;
}

static void set_escaper() {
    if (setjmp(escape) == 1) {
        printf("error has ocurred\n");
        exit(EXIT_SUCCESS);
    }
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

static void print_obj_inner(aq_obj_t obj) {
    aq_obj_type_t typ = aq_get_type(obj);
    switch (typ) {
    case AQ_OBJ_INT:
        printf("%ld", aq_decode_int(obj));
        return;
    case AQ_OBJ_BOOL:
        printf("#%c", aq_decode_bool(obj) ? 't' : 'f');
        return;
    case AQ_OBJ_CHAR:
        printf("'%c'", aq_decode_char(obj));
        return;
    case AQ_OBJ_NIL:
        printf("nil");
        return;
    case AQ_OBJ_PAIR:
        printf("(");
        print_obj_inner(aq_get_car(obj));
        printf(", ");
        print_obj_inner(aq_get_cdr(obj));
        printf(")");
        break;
    default:
        printf("<invalid type>");
        break;
    }
}

static void print_obj(aq_obj_t obj) {
    print_obj_inner(obj);
    printf("\n");
}

int main() {
    set_escaper();
    aq_state_t *aq = aq_init_state(libc_alloc);
    aq_set_panic(aq, error_handler);

    aq_obj_t c = aq_encode_char('a');
    aq_obj_t b = aq_encode_bool(false);
    aq_obj_t p1 = aq_encode_pair(aq, c, b);
    aq_obj_t n = aq_encode_nil();
    aq_obj_t p2 = aq_encode_pair(aq, p1, n);

    print_obj(p1);
    print_obj(p2);

    aq_obj_t fun = aq_init_test_closure(aq);
    aq_obj_t res = aq_execute_closure(aq, fun);
    print_obj(res);

    aq_deinit_state(aq);
    return 0;
}
