#include <stdio.h>
#include <stdlib.h>

#include "object.h"
#include "aqua.h"
#include "gc.h"

aq_obj_t aq_create_char(uint32_t cp) {
    aq_obj_t ret;
    OBJ_ENCODE_CHAR(ret, cp);
    return ret;
}

aq_obj_t aq_create_num(double num) {
    aq_obj_t ret;
    OBJ_ENCODE_NUM(ret, num);
    return ret;
}

aq_obj_t aq_create_nil(void) {
    aq_obj_t ret;
    OBJ_ENCODE_NIL(ret);
    return ret;
}

aq_obj_t aq_create_true() {
    aq_obj_t ret;
    OBJ_ENCODE_TRUE(ret);
    return ret;
}
aq_obj_t aq_create_false() {
    aq_obj_t ret;
    OBJ_ENCODE_FALSE(ret);
    return ret;
}

aq_obj_t aq_create_bool(bool b) {
    aq_obj_t ret;
    if (b)
        OBJ_ENCODE_TRUE(ret);
    else
        OBJ_ENCODE_FALSE(ret);
    return ret;
}

aq_obj_t aq_create_pair(aq_state_t *aq, aq_obj_t car, aq_obj_t cdr) {
    aq_pair_t *pair = GC_NEW(aq, aq_pair_t, HEAP_PAIR);
    pair->car = car;
    pair->cdr = cdr;
    aq_obj_t obj;
    OBJ_ENCODE_PAIR(obj, pair);
    return obj;
}

aq_obj_t aq_create_sym(aq_state_t *aq, const char *str, size_t sz) {
    aq_sym_t *sym = aq_intern_sym(aq, str, sz);
    aq_obj_t obj;
    OBJ_ENCODE_SYM(obj, sym);
    return obj;
}

uint32_t aq_get_char(aq_obj_t obj) {
    return obj.v.c;
}

double aq_get_num(aq_obj_t obj) {
    return obj.v.n;
}

aq_obj_t aq_get_car(aq_obj_t obj) {
    return OBJ_GET_CAR(obj);
}

aq_obj_t aq_get_cdr(aq_obj_t obj) {
    return OBJ_GET_CDR(obj);
}

const char *aq_get_sym(aq_obj_t obj, size_t *sz) {
    aq_sym_t *sym = OBJ_DECODE_HEAP(obj, aq_sym_t);
    *sz = sym->l;
    return sym->s;
}

aq_obj_type_t aq_get_type(aq_obj_t obj) {
    return obj.t;
}

aq_obj_t aq_create_table(aq_state_t *aq) {
    aq_tbl_t *tbl = GC_NEW(aq, aq_tbl_t, HEAP_TABLE);
    tbl->buckets_sz = 8;
    tbl->entries = 0;
    tbl->buckets =
        aq->alloc(NULL, 0, sizeof(aq_tbl_entry_t *) * tbl->buckets_sz);
    aq_obj_t ret;
    OBJ_ENCODE_TABLE(ret, tbl);
    return ret;
}

aq_tbl_t *aq_new_table(aq_state_t *aq, size_t init_buckets) {
    aq_tbl_t *tbl = GC_NEW(aq, aq_tbl_t, HEAP_TABLE);
    if (init_buckets == 0) {
        init_buckets = 8;
    }
    tbl->buckets_sz = init_buckets;
    tbl->entries = 0;
    tbl->buckets =
        aq->alloc(NULL, 0, sizeof(aq_tbl_entry_t *) * tbl->buckets_sz);
    return tbl;
}

static size_t hash_obj(aq_obj_t obj) {
    switch (obj.t) {
    case AQ_OBJ_CLOSURE:
    case AQ_OBJ_BIGNUM:
    case AQ_OBJ_PAIR:
    case AQ_OBJ_TABLE:
    case AQ_OBJ_ARRAY:
    case AQ_OBJ_CONTIN:
        return CAST(obj.v.h, size_t);
    case AQ_OBJ_SYM: {
        size_t total = 0;
        aq_sym_t *sym = OBJ_DECODE_SYM(obj);
        for (size_t i = 0; i < sym->l; i++) {
            total = (total << 4) + sym->s[i];
            size_t g = total & 0xf0000000;
            if (g != 0) {
                total = total ^ (g >> 24);
                total = total ^ g;
            }
        }
        return total;
    }
    case AQ_OBJ_NUM:
        return CAST(OBJ_DECODE_NUM(obj), size_t);
    case AQ_OBJ_TRUE:
        return 2;
    case AQ_OBJ_FALSE:
        return 1;
    case AQ_OBJ_NIL:
        return 0;
    case AQ_OBJ_CHAR:
        return OBJ_DECODE_CHAR(obj);
    default:
        printf("internal error: unimplemented hash case\n");
        exit(EXIT_FAILURE);
    }
}

bool obj_eq(aq_obj_t ob1, aq_obj_t ob2) {
    if (ob1.t != ob2.t)
        return false;
    switch (ob1.t) {
    case AQ_OBJ_NIL:
    case AQ_OBJ_TRUE:
    case AQ_OBJ_FALSE:
        return true;
    case AQ_OBJ_NUM:
        return ob1.v.n == ob2.v.n;
    case AQ_OBJ_CHAR:
        return ob1.v.c == ob2.v.c;
    case AQ_OBJ_SYM:
    case AQ_OBJ_CLOSURE:
    case AQ_OBJ_PAIR:
    case AQ_OBJ_CONTIN:
    case AQ_OBJ_BIGNUM:
    case AQ_OBJ_ARRAY:
    case AQ_OBJ_TABLE:
        return ob1.v.h == ob2.v.h;
    }
    return false;
}

aq_obj_t aq_table_get(aq_state_t *aq, aq_obj_t tbl_obj, aq_obj_t key) {
    if (OBJ_IS_TABLE(tbl_obj)) {
        aq_tbl_t *tbl = OBJ_DECODE_HEAP(tbl_obj, aq_tbl_t);
        size_t idx = hash_obj(key) % tbl->buckets_sz;
        for (aq_tbl_entry_t *entry = tbl->buckets[idx]; entry;
             entry = entry->n) {
            if (obj_eq(entry->k, key)) {
                return entry->v;
            }
        }
        aq_obj_t ret;
        OBJ_ENCODE_NIL(ret);
        return ret;
    }
    aq->panic(aq, AQ_ERR_NOT_TABLE);
    return tbl_obj; /* this will never be reached */
}

void aq_table_set(aq_state_t *aq, aq_obj_t tbl_obj, aq_obj_t key,
                  aq_obj_t val) {
    if (OBJ_IS_TABLE(tbl_obj)) {
        aq_tbl_t *tbl = OBJ_DECODE_HEAP(tbl_obj, aq_tbl_t);
        size_t idx = hash_obj(key) % tbl->buckets_sz;
        for (aq_tbl_entry_t *entry = tbl->buckets[idx]; entry;
             entry = entry->n) {
            if (obj_eq(entry->k, key)) {
                entry->v = val;
                return;
            }
        }
        aq_tbl_entry_t *new_entry = aq->alloc(NULL, 0, sizeof(aq_tbl_entry_t));
        new_entry->n = tbl->buckets[idx];
        tbl->buckets[idx] = new_entry;
        new_entry->k = key;
        new_entry->v = val;
        tbl->entries++;
    } else {
        aq->panic(aq, AQ_ERR_NOT_TABLE);
    }
}
