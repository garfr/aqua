#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "aqua.h"

static const char *err_names[] = {
    [AQ_ERR_OOM] = "Out of Memory",
    [AQ_ERR_INVALID_ARITH] = "Invalid Arithmetic (Type Error)",
    [AQ_ERR_NOT_PAIR] = "Not Pair (Type Error)",
    [AQ_ERR_NOT_TABLE] = "Not Table (Type Error)",
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

static void print_obj_inner(aq_obj_t obj) {
    aq_obj_type_t typ = aq_get_type(obj);
    switch (typ) {
    case AQ_OBJ_INT:
        printf("%ld", aq_get_int(obj));
        return;
    case AQ_OBJ_BOOL:
        printf("#%c", aq_get_bool(obj) ? 't' : 'f');
        return;
    case AQ_OBJ_CHAR:
        printf("'%c'", aq_get_char(obj));
        return;
    case AQ_OBJ_NIL:
        printf("nil");
        return;
    case AQ_OBJ_SYM: {
        size_t sz;
        const char *sym = aq_get_sym(obj, &sz);
        printf("%.*s", (int)sz, sym);
        break;
    }
    case AQ_OBJ_PAIR:
        printf("(");
        print_obj_inner(aq_get_car(obj));
        printf(", ");
        print_obj_inner(aq_get_cdr(obj));
        printf(")");
        break;
    case AQ_OBJ_TABLE:
        printf("<table>");
        break;
    default:
        printf("<invalid type> %d", typ);
        break;
    }
}

static void print_obj(aq_obj_t obj) {
    print_obj_inner(obj);
    printf("\n");
}

int main() {
    aq_state_t *aq = aq_init_state(libc_alloc);
    aq_set_panic(aq, error_handler);

    printf("%ld bytes\n", aq_get_mem_used(aq));

    aq_obj_t fun = aq_init_test_closure(aq);
    aq_obj_t res = aq_execute_closure(aq, fun);
    print_obj(res);

    printf("%ld bytes\n", aq_get_mem_used(aq));
    aq_deinit_state(aq);
    return 0;
}
