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
