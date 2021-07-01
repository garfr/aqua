#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "object.h"

#define HEAP_SZ 65536
#define VARS_SZ 1024
#define CALLS_SZ 1024
#define BUCKETS_SZ 64
#define FREEZE_SZ 9

aq_state_t *aq_init_state(aq_alloc_t alloc) {
    aq_state_t *aq = alloc(NULL, 0, sizeof(aq_state_t));
    aq->alloc = alloc;
    aq->gc_root = NULL;

    aq->vars = alloc(NULL, 0, sizeof(aq_obj_t) * VARS_SZ);
    aq->vars_sz = VARS_SZ;
    for (size_t i = 0; i < aq->vars_sz; i++) {
        OBJ_ENCODE_NIL(aq->vars[i]);
    }

    aq->calls = alloc(NULL, 0, sizeof(aq_call_t) * CALLS_SZ);
    aq->calls_sz = CALLS_SZ;
    aq->cur_call = aq->calls;

    aq->syms.buckets_sz = BUCKETS_SZ;
    aq->syms.syms = 0;
    aq->syms.buckets = alloc(NULL, 0, sizeof(aq_sym_t *) * aq->syms.buckets_sz);

    aq->panic = NULL;

    aq->frozen = aq->alloc(NULL, 0, FREEZE_SZ * sizeof(aq_obj_t));
    aq->frozen_cap = FREEZE_SZ;
    aq->frozen_sz = 0;

    aq_tbl_t *tbl = aq_new_table(aq, 16);
    OBJ_ENCODE_TABLE(aq->g, tbl);
    return aq;
}

void aq_deinit_state(aq_state_t *aq) {
    aq->alloc(aq->vars, sizeof(aq_obj_t) * aq->vars_sz, 0);
    aq->alloc(aq->calls, sizeof(aq_call_t) * aq->calls_sz, 0);

    aq_free_all(aq);

    aq->alloc(aq->syms.buckets, sizeof(aq_sym_t *) * aq->syms.buckets_sz, 0);
    aq->alloc(aq->frozen, 0, 0);
    aq->alloc(aq, sizeof(aq_state_t), 0);
}

void aq_set_panic(aq_state_t *aq, aq_panic_t panic) {
    aq->panic = panic;
}

void aq_panic(aq_state_t *aq, aq_err_t err) {
    if (aq->panic) {
        aq->panic(aq, err); /* call the user defined panic function */
    }
    exit(EXIT_FAILURE);
}

/* PJW hash */
size_t hash_string(const char *str, size_t sz) {
    size_t total = 0;
    for (size_t i = 0; i < sz; i++) {
        total = (total << 4) + str[i];
        size_t g = total & 0xf0000000;
        if (g != 0) {
            total = total ^ (g >> 24);
            total = total ^ g;
        }
    }
    return total;
}

aq_sym_t *aq_intern_sym(aq_state_t *aq, const char *str, size_t sz) {
    size_t idx = hash_string(str, sz) % aq->syms.buckets_sz;
    aq_sym_t *sym = aq->syms.buckets[idx];
    for (; sym; sym = sym->next) {
        if (sz == sym->l && strncmp(str, sym->s, sz) == 0)
            return sym;
    }
    aq_sym_t *new_sym =
        GC_NEW_BYTES(aq, sizeof(aq_sym_t) + sz, aq_sym_t, HEAP_SYM);
    new_sym->l = sz;
    memcpy(new_sym->s, str, sz);
    new_sym->next = aq->syms.buckets[idx];
    aq->syms.buckets[idx] = new_sym;
    return new_sym;
}

void aq_freeze_var(aq_state_t *aq, aq_obj_t *obj) {
    for (size_t i = 0; i < aq->frozen_sz; i++) {
        if (aq->frozen[i] == obj) {
            return;
        }
    }

    for (size_t i = 0; i < aq->frozen_sz; i++) {
        if (aq->frozen[i] == NULL) {
            aq->frozen[i] = obj;
        }
    }

    if (aq->frozen_sz + 1 >= aq->frozen_cap) {
        aq->frozen = aq->alloc(aq->frozen, aq->frozen_cap * sizeof(aq_obj_t *),
                               aq->frozen_cap * 2 * sizeof(aq_obj_t *));
        aq->frozen_cap *= 2;
    }
    aq->frozen[aq->frozen_sz++] = obj;
}

void aq_unfreeze_var(aq_state_t *aq, aq_obj_t *obj) {
    for (size_t i = 0; i < aq->frozen_sz; i++) {
        if (aq->frozen[i] == obj) {
            aq->frozen[i] = NULL;
        }
    }
}
