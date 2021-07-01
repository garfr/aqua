#ifndef OBJECT_H
#define OBJECT_H
/* utilities for encoding and decoding objects */

#include <stdint.h>

#include "types.h"
#include "aqua.h"
#include "helpers.h"

#define OBJ_IS_NIL(obj) ((obj).t == AQ_OBJ_NIL)

#define OBJ_IS(obj, tag) ((obj).t == (tag))

#define OBJ_IS_CHAR(obj) OBJ_IS(obj, AQ_OBJ_CHAR)
#define OBJ_IS_TRUE(obj) OBJ_IS(obj, AQ_OBJ_TRUE)
#define OBJ_IS_FALSE(obj) OBJ_IS(obj, AQ_OBJ_FALSE)
#define OBJ_IS_NUM(obj) OBJ_IS(obj, AQ_OBJ_NUM)

#define OBJ_IS_HEAP(obj, tag) OBJ_IS(obj, OBJ_HEAP_MASK, tag)

#define OBJ_IS_HEAP_ANY(obj) ((obj).t >= AQ_OBJ_PAIR && (obj).t <= AQ_OBJ_SYM)

#define OBJ_IS_PAIR(obj) (OBJ_IS(obj, AQ_OBJ_PAIR))
#define OBJ_IS_CLOSURE(obj) (OBJ_IS(obj, AQ_OBJ_CLOSURE))
#define OBJ_IS_ARRAY(obj) (OBJ_IS(obj, AQ_OBJ_ARRAY))
#define OBJ_IS_TABLE(obj) (OBJ_IS(obj, AQ_OBJ_TABLE))
#define OBJ_IS_CONTIN(obj) (OBJ_IS(obj, AQ_OBJ_CONTIN))
#define OBJ_IS_BIGNUM(obj) (OBJ_IS(obj, AQ_OBJ_BIGNUM))
#define OBJ_IS_SYM(obj) (OBJ_IS(obj, AQ_OBJ_SYM))

#define OBJ_ENCODE_LIT(val, tag) OBJ_ENCODE(val, OBJ_LIT_SHIFT, tag)

#define OBJ_ENCODE_CHAR(val, cp) (val.v.c = cp, val.t = AQ_OBJ_CHAR)
#define OBJ_ENCODE_TRUE(val) (val.t = AQ_OBJ_TRUE)
#define OBJ_ENCODE_FALSE(val) (val.t = AQ_OBJ_FALSE)
#define OBJ_ENCODE_NIL(val) (val.t = AQ_OBJ_NIL)

#define OBJ_ENCODE_NUM(val, num) (val.v.n = num, val.t = AQ_OBJ_NUM)

#define OBJ_ENCODE_HEAP(val, ptr, tag)                                         \
    (val.v.h = (aq_heap_obj_t *)ptr, val.t = tag)

#define OBJ_ENCODE_PAIR(val, ptr) OBJ_ENCODE_HEAP(val, ptr, AQ_OBJ_PAIR)
#define OBJ_ENCODE_CLOSURE(val, ptr) OBJ_ENCODE_HEAP(val, ptr, AQ_OBJ_CLOSURE)
#define OBJ_ENCODE_ARRAY(val, ptr) OBJ_ENCODE_HEAP(val, ptr, AQ_OBJ_ARRAY)
#define OBJ_ENCODE_TABLE(val, ptr) OBJ_ENCODE_HEAP(val, ptr, AQ_OBJ_TABLE)
#define OBJ_ENCODE_CONTIN(val, ptr) OBJ_ENCODE_HEAP(val, ptr, AQ_OBJ_CONTIN)
#define OBJ_ENCODE_BIGNUM(val, ptr) OBJ_ENCODE_HEAP(val, ptr, AQ_OBJ_BIGNUM)
#define OBJ_ENCODE_SYM(val, ptr) OBJ_ENCODE_HEAP(val, ptr, AQ_OBJ_SYM)

#define OBJ_DECODE_HEAP(obj, type) CAST(obj.v.h, type *)

#define OBJ_DECODE_PAIR(obj) OBJ_DECODE_HEAP(obj, aq_pair_t)
#define OBJ_DECODE_CLOSURE(obj) OBJ_DECODE_HEAP(obj, aq_closure_t)
#define OBJ_DECODE_SYM(obj) OBJ_DECODE_HEAP(obj, aq_sym_t)

#define OBJ_DECODE_NUM(obj) (obj.v.n)
#define OBJ_DECODE_CHAR(obj) (obj.v.c)

#define OBJ_GET_CAR(obj) (OBJ_DECODE_PAIR(obj)->car)
#define OBJ_GET_CDR(obj) (OBJ_DECODE_PAIR(obj)->cdr)

aq_tbl_t *aq_new_table(aq_state_t *state, size_t init_buckets);

#endif

