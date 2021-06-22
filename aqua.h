#ifndef AQUA_H
#define AQUA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define AQUA_VERSION "0.1"

typedef void *(*aq_alloc_t)(void *, size_t, size_t);

typedef struct aq_state_t aq_state_t;

typedef enum { AQ_ERR_OOM, AQ_ERR_INVALID_ARITH, AQ_ERR_NOT_PAIR } aq_err_t;

typedef int (*aq_panic_t)(aq_state_t *, aq_err_t err);

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
    AQ_OBJ_SYM,
} aq_obj_type_t;

aq_state_t *aq_init_state(aq_alloc_t alloc);
void aq_deinit_state(aq_state_t *aq);

void aq_print_version();

aq_obj_t aq_create_char(uint32_t cp);
aq_obj_t aq_create_int(int64_t num);
aq_obj_t aq_create_nil(void);
aq_obj_t aq_create_bool(bool b);
aq_obj_t aq_create_pair(aq_state_t *aq, aq_obj_t car, aq_obj_t cdr);
aq_obj_t aq_create_sym(aq_state_t *aq, const char *str, size_t sz);

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
bool aq_is_sym(aq_obj_t obj);

uint32_t aq_get_char(aq_obj_t obj);
int64_t aq_get_int(aq_obj_t obj);
bool aq_get_bool(aq_obj_t obj);

aq_obj_t aq_get_car(aq_obj_t obj);
aq_obj_t aq_get_cdr(aq_obj_t obj);
const char *aq_get_sym(aq_obj_t obj, size_t *sz);

aq_obj_type_t aq_get_type(aq_obj_t obj);

aq_obj_t aq_execute_closure(aq_state_t *aq, aq_obj_t obj);
aq_obj_t aq_init_test_closure(aq_state_t *aq);

void aq_set_panic(aq_state_t *aq, aq_panic_t panic);

size_t aq_get_mem_used(aq_state_t *aq);

#endif
