#ifndef AQUA_H
#define AQUA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define AQUA_VERSION "0.1"

typedef void *(*aq_alloc_t)(void *, size_t, size_t);

typedef struct aq_state_t aq_state_t;

typedef uint64_t aq_obj_t;

typedef enum {
    AQ_OBJ_NIL,
    AQ_OBJ_CHAR,
    AQ_OBJ_INT,
    AQ_OBJ_BOOL,
    AQ_OBJ_PAIR,
    AQ_OBJ_CLOSURE,
    AQ_OBJ_ARRAY,
    AQ_OBJ_TABLE,
    AQ_OBJ_CONTIN,
    AQ_OBJ_BIGNUM,
} aq_obj_type_t;

aq_state_t *aq_init_state(aq_alloc_t alloc);
void aq_deinit_state(aq_state_t *aq);

void aq_print_version();

aq_obj_t aq_encode_char(uint32_t cp);
aq_obj_t aq_encode_int(int64_t num);
aq_obj_t aq_encode_nil(void);
aq_obj_t aq_encode_bool(bool b);
aq_obj_t aq_encode_pair(aq_state_t *aq, aq_obj_t car, aq_obj_t cdr);

bool aq_is_char(aq_obj_t obj);
bool aq_is_int(aq_obj_t obj);
bool aq_is_nil(aq_obj_t obj);
bool aq_is_bool(aq_obj_t obj);
bool aq_is_pair(aq_obj_t obj);
bool aq_is_closure(aq_obj_t obj);
bool aq_is_array(aq_obj_t obj);
bool aq_is_table(aq_obj_t obj);
bool aq_is_contin(aq_obj_t obj);
bool aq_is_bignum(aq_obj_t obj);

uint32_t aq_decode_char(aq_obj_t obj);
int64_t aq_decode_int(aq_obj_t obj);
bool aq_decode_bool(aq_obj_t obj);

aq_obj_t aq_get_car(aq_obj_t obj);
aq_obj_t aq_get_cdr(aq_obj_t obj);

aq_obj_type_t aq_get_type(aq_obj_t obj);

#endif
