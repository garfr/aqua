#include "object.h"
#include "aqua.h"
#include "gc.h"

aq_obj_t aq_encode_char(uint32_t cp) {
    return (cp << OBJ_LIT_SHIFT) | OBJ_CHAR_TAG;
}
aq_obj_t aq_encode_int(int64_t num) {
    return (num << OBJ_INT_SHIFT) | OBJ_INT_TAG;
}

aq_obj_t aq_encode_nil(void) {
    return OBJ_NIL_VAL;
}

aq_obj_t aq_encode_bool(bool b) {
    return (b << OBJ_LIT_SHIFT) | OBJ_BOOL_TAG;
}

aq_obj_t aq_encode_pair(aq_state_t *aq, aq_obj_t car, aq_obj_t cdr) {
    aq_pair_t *pair = aq_gc_alloc(&aq->heap, sizeof(aq_pair_t));
    pair->car = car;
    pair->cdr = cdr;
    return (((uint64_t)pair) & ~OBJ_HEAP_MASK) | OBJ_PAIR_TAG;
}

aq_obj_t aq_encode_closure(aq_closure_t *c) {
    return (((uint64_t)c) & ~OBJ_HEAP_MASK) | OBJ_CLOSURE_TAG;
}

bool aq_is_char(aq_obj_t obj) {
    return (obj & OBJ_LIT_MASK) == OBJ_CHAR_TAG;
}

bool aq_is_int(aq_obj_t obj) {
    return (obj & OBJ_INT_MASK) == OBJ_INT_TAG;
}

bool aq_is_nil(aq_obj_t obj) {
    return obj == OBJ_NIL_VAL;
}

bool aq_is_bool(aq_obj_t obj) {
    return (obj & OBJ_LIT_MASK) == OBJ_BOOL_TAG;
}

bool aq_is_pair(aq_obj_t obj) {
    return (obj & OBJ_HEAP_MASK) == OBJ_PAIR_TAG;
}

bool aq_is_closure(aq_obj_t obj) {
    return (obj & OBJ_HEAP_MASK) == OBJ_CLOSURE_TAG;
}

bool aq_is_array(aq_obj_t obj) {
    return (obj & OBJ_HEAP_MASK) == OBJ_ARRAY_TAG;
}

bool aq_is_table(aq_obj_t obj) {
    return (obj & OBJ_HEAP_MASK) == OBJ_TABLE_TAG;
}

bool aq_is_contin(aq_obj_t obj) {
    return (obj & OBJ_HEAP_MASK) == OBJ_CONTIN_TAG;
}

bool aq_is_bignum(aq_obj_t obj) {
    return (obj & OBJ_HEAP_MASK) == OBJ_BIGNUM_TAG;
}

uint32_t aq_decode_char(aq_obj_t obj) {
    return obj >> OBJ_LIT_SHIFT;
}

int64_t aq_decode_int(aq_obj_t obj) {
    return obj >> OBJ_INT_SHIFT;
}

bool aq_decode_bool(aq_obj_t obj) {
    return obj >> OBJ_LIT_SHIFT;
}

aq_obj_t aq_get_car(aq_obj_t obj) {
    aq_pair_t *pair = (aq_pair_t *)GET_HEAP_PTR(obj);
    return pair->car;
}

aq_obj_t aq_get_cdr(aq_obj_t obj) {
    aq_pair_t *pair = (aq_pair_t *)GET_HEAP_PTR(obj);
    return pair->cdr;
}

/* TODO: implement this more efficiently */
aq_obj_type_t aq_get_type(aq_obj_t obj) {
    if ((obj & OBJ_INT_MASK) == 0) {
        return AQ_OBJ_INT;
    }
    if ((obj & 0xF) == 0xF) {
        if (aq_is_int(obj)) {
            return AQ_OBJ_INT;
        } else if (aq_is_char(obj)) {
            return AQ_OBJ_CHAR;
        } else if (aq_is_nil(obj)) {
            return AQ_OBJ_NIL;
        } else if (aq_is_bool(obj)) {
            return AQ_OBJ_BOOL;
        }
    } else {
        if (aq_is_pair(obj)) {
            return AQ_OBJ_PAIR;
        } else if (aq_is_closure(obj)) {
            return AQ_OBJ_CLOSURE;
        } else if (aq_is_array(obj)) {
            return AQ_OBJ_ARRAY;
        } else if (aq_is_table(obj)) {
            return AQ_OBJ_TABLE;
        } else if (aq_is_contin(obj)) {
            return AQ_OBJ_CONTIN;
        } else if (aq_is_bignum(obj)) {
            return AQ_OBJ_BIGNUM;
        }
    }
    return -1;
}
