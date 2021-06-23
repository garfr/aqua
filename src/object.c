#include "object.h"
#include "aqua.h"
#include "gc.h"

aq_obj_t aq_create_char(uint32_t cp) {
    return OBJ_ENCODE_CHAR(cp);
}
aq_obj_t aq_create_int(int64_t num) {
    return OBJ_ENCODE_INT(num);
}

aq_obj_t aq_create_nil(void) {
    return OBJ_NIL_VAL;
}

aq_obj_t aq_create_bool(bool b) {
    return OBJ_ENCODE_BOOL(b);
}

aq_obj_t aq_create_pair(aq_state_t *aq, aq_obj_t car, aq_obj_t cdr) {
    aq_pair_t *pair = GC_NEW(aq, aq_pair_t);
    pair->tt = HEAP_PAIR;
    pair->car = car;
    pair->cdr = cdr;
    return OBJ_ENCODE_PAIR(pair);
}

aq_obj_t aq_create_sym(aq_state_t *aq, const char *str, size_t sz) {
    aq_sym_t *sym = aq_intern_sym(aq, str, sz);
    return OBJ_ENCODE_SYM(sym);
}

aq_obj_t aq_encode_closure(aq_closure_t *c) {
    return OBJ_ENCODE_CLOSURE(c);
}

bool aq_is_char(aq_obj_t obj) {
    return OBJ_IS_CHAR(obj);
}

bool aq_is_int(aq_obj_t obj) {
    return OBJ_IS_INT(obj);
}

bool aq_is_nil(aq_obj_t obj) {
    return OBJ_IS_NIL(obj);
}

bool aq_is_bool(aq_obj_t obj) {
    return OBJ_IS_BOOL(obj);
}

bool aq_is_pair(aq_obj_t obj) {
    return OBJ_IS_PAIR(obj);
}

bool aq_is_closure(aq_obj_t obj) {
    return OBJ_IS_CLOSURE(obj);
}

bool aq_is_array(aq_obj_t obj) {
    return OBJ_IS_ARRAY(obj);
}

bool aq_is_table(aq_obj_t obj) {
    return OBJ_IS_TABLE(obj);
}

bool aq_is_contin(aq_obj_t obj) {
    return OBJ_IS_CONTIN(obj);
}

bool aq_is_bignum(aq_obj_t obj) {
    return OBJ_IS_BIGNUM(obj);
}

bool aq_is_sym(aq_obj_t obj) {
    return OBJ_IS_SYM(obj);
}

uint32_t aq_get_char(aq_obj_t obj) {
    return OBJ_DECODE_CHAR(obj);
}

int64_t aq_get_int(aq_obj_t obj) {
    return OBJ_DECODE_INT(obj);
}

bool aq_get_bool(aq_obj_t obj) {
    return OBJ_DECODE_BOOL(obj);
}

aq_obj_t aq_get_car(aq_obj_t obj) {
    return OBJ_GET_CAR(obj);
}

aq_obj_t aq_get_cdr(aq_obj_t obj) {
    return OBJ_GET_CDR(obj);
}

const char *aq_get_sym(aq_obj_t obj, size_t *sz) {
    aq_sym_t *sym = OBJ_DECODE_HEAP(obj, aq_sym_t *);
    *sz = sym->l;
    return sym->s;
}

aq_obj_type_t aq_get_type(aq_obj_t obj) {
    if ((obj & OBJ_INT_MASK) == 0) {
        return AQ_OBJ_INT;
    }
    if ((obj & 0xF) == 0xF) {
        if (OBJ_IS_INT(obj)) {
            return AQ_OBJ_INT;
        } else if (OBJ_IS_CHAR(obj)) {
            return AQ_OBJ_CHAR;
        } else if (OBJ_IS_NIL(obj)) {
            return AQ_OBJ_NIL;
        } else if (OBJ_IS_BOOL(obj)) {
            return AQ_OBJ_BOOL;
        }
    } else {
        if (aq_is_pair(obj)) {
            return AQ_OBJ_PAIR;
        } else if (OBJ_IS_PAIR(obj)) {
            return AQ_OBJ_PAIR;
        } else if (OBJ_IS_SYM(obj)) {
            return AQ_OBJ_SYM;
        } else if (OBJ_IS_CLOSURE(obj)) {
            return AQ_OBJ_CLOSURE;
        } else if (OBJ_IS_ARRAY(obj)) {
            return AQ_OBJ_ARRAY;
        } else if (OBJ_IS_TABLE(obj)) {
            return AQ_OBJ_TABLE;
        } else if (OBJ_IS_CONTIN(obj)) {
            return AQ_OBJ_CONTIN;
        }
    }
    return -1;
}

aq_tbl_t *aq_new_table(aq_state_t *aq, size_t init_buckets) {
    aq_tbl_t *tbl = GC_NEW(aq, aq_tbl_t);
    if (init_buckets == 0) {
        init_buckets = 8;
    }
    tbl->buckets_sz = init_buckets;
    tbl->entries = 0;
    tbl->buckets = GC_NEW_ARRAY(aq, aq_tbl_entry_t *, tbl->buckets_sz);
    return tbl;
}
