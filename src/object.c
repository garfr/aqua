#include "object.h"
#include "aqua.h"

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

uint32_t aq_decode_char(aq_obj_t obj) {
    return obj >> OBJ_LIT_SHIFT;
}

int64_t aq_decode_int(aq_obj_t obj) {
    return obj >> OBJ_INT_SHIFT;
}

bool aq_decode_bool(aq_obj_t obj) {
    return obj >> OBJ_LIT_SHIFT;
}

aq_obj_type_t aq_get_type(aq_obj_t obj) {
    if (aq_is_int(obj)) {
        return AQ_OBJ_INT;
    } else if (aq_is_char(obj)) {
        return AQ_OBJ_CHAR;
    } else if (aq_is_nil(obj)) {
        return AQ_OBJ_NIL;
    } else if (aq_is_bool(obj)) {
        return AQ_OBJ_BOOL;
    }
    return -1;
}
