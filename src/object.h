#ifndef OBJECT_H
#define OBJECT_H
/* utilities for encoding and decoding objects */

#include <stdint.h>

#include "types.h"
#include "aqua.h"
#include "helpers.h"

#define OBJ_NIL_VAL 0xF /* 0b001111 */

#define OBJ_INT_TAG 0x0
#define OBJ_INT_MASK 0x7 /* 0b111 */
#define OBJ_INT_SHIFT 3

#define OBJ_LIT_SHIFT 6
#define OBJ_LIT_MASK 0x3F /* 0b111111 */

#define OBJ_BOOL_TAG 0x2F /* 0b101111 */
#define OBJ_CHAR_TAG 0x1F /* 0b011111 */

#define OBJ_HEAP_SHIFT 4
#define OBJ_HEAP_MASK 0xF /* 0b1111 */

#define OBJ_PAIR_TAG 0x1    /* 0b0001 */
#define OBJ_CLOSURE_TAG 0x2 /* 0b0010 */
#define OBJ_ARRAY_TAG 0x3   /* 0b0011 */
#define OBJ_TABLE_TAG 0x4   /* 0b0100 */
#define OBJ_CONTIN_TAG 0x5  /* 0b0101 */
#define OBJ_BIGNUM_TAG 0x6  /* 0b0110 */
#define OBJ_SYM_TAG 0x7     /* 0b0111 */

#define OBJ_IS_NIL(obj) ((obj) == OBJ_NIL_VAL)

#define OBJ_IS(obj, mask, tag) (((obj) & (mask)) == (tag))

#define OBJ_IS_LIT(obj, tag) OBJ_IS(obj, OBJ_LIT_MASK, tag)

#define OBJ_IS_CHAR(obj) OBJ_IS_LIT(obj, OBJ_CHAR_TAG)
#define OBJ_IS_BOOL(obj) OBJ_IS_LIT(obj, OBJ_BOOL_TAG)
#define OBJ_IS_INT(obj) OBJ_IS(obj, OBJ_INT_MASK, OBJ_INT_TAG)

#define OBJ_IS_HEAP(obj, tag) OBJ_IS(obj, OBJ_HEAP_MASK, tag)

#define OBJ_IS_HEAP_ANY(obj)                                                   \
    (((obj) & (OBJ_HEAP_MASK)) >= OBJ_PAIR_TAG) &&                             \
        (((obj) & (OBJ_HEAP_MASK)) <= OBJ_SYM_TAG)

#define OBJ_IS_PAIR(obj) (OBJ_IS_HEAP(obj, OBJ_PAIR_TAG))
#define OBJ_IS_CLOSURE(obj) (OBJ_IS_HEAP(obj, OBJ_CLOSURE_TAG))
#define OBJ_IS_ARRAY(obj) (OBJ_IS_HEAP(obj, OBJ_ARRAY_TAG))
#define OBJ_IS_TABLE(obj) (OBJ_IS_HEAP(obj, OBJ_TABLE_TAG))
#define OBJ_IS_CONTIN(obj) (OBJ_IS_HEAP(obj, OBJ_CONTIN_TAG))
#define OBJ_IS_BIGNUM(obj) (OBJ_IS_HEAP(obj, OBJ_BIGNUM_TAG))
#define OBJ_IS_SYM(obj) (OBJ_IS_HEAP(obj, OBJ_SYM_TAG))

#define OBJ_ENCODE(val, shift, tag) ((val << (shift)) | (tag))
#define OBJ_ENCODE_LIT(val, tag) OBJ_ENCODE(val, OBJ_LIT_SHIFT, tag)

#define OBJ_ENCODE_CHAR(val) OBJ_ENCODE_LIT(val, OBJ_CHAR_TAG)
#define OBJ_ENCODE_BOOL(val) OBJ_ENCODE_LIT(val, OBJ_BOOL_TAG)

#define OBJ_ENCODE_INT(val) OBJ_ENCODE(val, OBJ_INT_SHIFT, OBJ_INT_TAG)

#define OBJ_ENCODE_HEAP(ptr, tag)                                              \
    ((CAST(ptr, uint64_t) & (~OBJ_HEAP_MASK)) | (tag))

#define OBJ_ENCODE_PAIR(ptr) OBJ_ENCODE_HEAP(ptr, OBJ_PAIR_TAG)
#define OBJ_ENCODE_CLOSURE(ptr) OBJ_ENCODE_HEAP(ptr, OBJ_CLOSURE_TAG)
#define OBJ_ENCODE_ARRAY(ptr) OBJ_ENCODE_HEAP(ptr, OBJ_ARRAY_TAG)
#define OBJ_ENCODE_TABLE(ptr) OBJ_ENCODE_HEAP(ptr, OBJ_TABLE_TAG)
#define OBJ_ENCODE_CONTIN(ptr) OBJ_ENCODE_HEAP(ptr, OBJ_CONTIN_TAG)
#define OBJ_ENCODE_BIGNUM(ptr) OBJ_ENCODE_HEAP(ptr, OBJ_BIGNUM_TAG)
#define OBJ_ENCODE_SYM(ptr) OBJ_ENCODE_HEAP(ptr, OBJ_SYM_TAG)

#define OBJ_DECODE_LIT(obj, shift) ((obj) >> (shift))

#define OBJ_DECODE_HEAP(obj, type) CAST(obj & ~OBJ_HEAP_MASK, type)

#define OBJ_DECODE_PAIR(obj) OBJ_DECODE_HEAP(obj, aq_pair_t *)
#define OBJ_DECODE_CLOSURE(obj) OBJ_DECODE_HEAP(obj, aq_closure_t *)
#define OBJ_DECODE_SYM(obj) OBJ_DECODE_HEAP(obj, aq_sym_t *)

#define OBJ_DECODE_INT(obj) OBJ_DECODE_LIT(obj, OBJ_INT_SHIFT)
#define OBJ_DECODE_CHAR(obj) OBJ_DECODE_LIT(obj, OBJ_LIT_SHIFT)
#define OBJ_DECODE_BOOL(obj) OBJ_DECODE_LIT(obj, OBJ_LIT_SHIFT)

#define OBJ_GET_CAR(obj) (OBJ_DECODE_PAIR(obj)->car)
#define OBJ_GET_CDR(obj) (OBJ_DECODE_PAIR(obj)->cdr)

aq_obj_t aq_encode_closure(aq_closure_t *c);

aq_tbl_t *aq_new_table(aq_state_t *state, size_t init_buckets);

#endif