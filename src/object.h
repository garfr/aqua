#ifndef OBJECT_H
#define OBJECT_H
/* utilities for encoding and decoding objects */

#include <stdint.h>

#include "types.h"
#include "aqua.h"

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

#define GET_HEAP_PTR(obj) (obj & ~OBJ_HEAP_MASK);

aq_obj_t aq_encode_closure(aq_closure_t *c);

#endif