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
} aq_obj_type_t;

aq_state_t *aq_init_state(aq_alloc_t alloc);
void aq_deinit_state(aq_state_t *aq);

void aq_print_version();

aq_obj_t aq_encode_char(uint32_t cp);
aq_obj_t aq_encode_int(int64_t num);
aq_obj_t aq_encode_nil(void);
aq_obj_t aq_encode_bool(bool b);

bool aq_is_char(aq_obj_t obj);
bool aq_is_int(aq_obj_t obj);
bool aq_is_nil(aq_obj_t obj);
bool aq_is_bool(aq_obj_t obj);

uint32_t aq_decode_char(aq_obj_t obj);
int64_t aq_decode_int(aq_obj_t obj);
bool aq_decode_bool(aq_obj_t obj);

aq_obj_type_t aq_get_type(aq_obj_t obj);

#endif
