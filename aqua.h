#ifndef AQUA_H
#define AQUA_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define AQUA_VERSION "0.1"

typedef void *(*aq_alloc_t)(void *, size_t, size_t);

typedef struct aq_state_t aq_state_t;

typedef enum {
    AQ_ERR_OOM,
    AQ_ERR_INVALID_ARITH,
    AQ_ERR_NOT_PAIR,
    AQ_ERR_NOT_TABLE,
    AQ_ERR_INVALID_COMP,
    AQ_ERR_INVALID_FILENAME,
} aq_err_t;

typedef int (*aq_panic_t)(aq_state_t *, aq_err_t err);

typedef struct aq_heap_obj_t aq_heap_obj_t;

typedef enum {
    AQ_OBJ_NIL,
    AQ_OBJ_CHAR,
    AQ_OBJ_NUM,
    AQ_OBJ_TRUE,
    AQ_OBJ_FALSE,
    AQ_OBJ_PAIR,
    AQ_OBJ_CLOSURE,
    AQ_OBJ_ARRAY,
    AQ_OBJ_TABLE,
    AQ_OBJ_CONTIN,
    AQ_OBJ_BIGNUM,
    AQ_OBJ_SYM,
} aq_obj_type_t;

typedef struct {
    aq_obj_type_t t;
    union {
        double n;
        uint32_t c;
        aq_heap_obj_t *h;
    } v;
} aq_obj_t;

aq_state_t *aq_init_state(aq_alloc_t alloc);
void aq_deinit_state(aq_state_t *aq);

void aq_print_version();

aq_obj_t aq_create_char(uint32_t cp);
aq_obj_t aq_create_num(double num);
aq_obj_t aq_create_nil(void);
aq_obj_t aq_create_bool(bool b);
aq_obj_t aq_create_true();
aq_obj_t aq_create_false();
aq_obj_t aq_create_pair(aq_state_t *aq, aq_obj_t car, aq_obj_t cdr);
aq_obj_t aq_create_sym(aq_state_t *aq, const char *str, size_t sz);
aq_obj_t aq_create_table(aq_state_t *aq);

aq_obj_t aq_table_get(aq_state_t *aq, aq_obj_t tbl, aq_obj_t key);
void aq_table_set(aq_state_t *aq, aq_obj_t tbl, aq_obj_t key, aq_obj_t val);

uint32_t aq_get_char(aq_obj_t obj);
double aq_get_num(aq_obj_t obj);
bool aq_get_bool(aq_obj_t obj);

aq_obj_t aq_get_car(aq_obj_t obj);
aq_obj_t aq_get_cdr(aq_obj_t obj);
const char *aq_get_sym(aq_obj_t obj, size_t *sz);

aq_obj_type_t aq_get_type(aq_obj_t obj);

void aq_set_panic(aq_state_t *aq, aq_panic_t panic);

void aq_collect_garbage(aq_state_t *aq);

aq_obj_t aq_read_file(aq_state_t *aq, const char *str);
aq_obj_t aq_read_string(aq_state_t *aq, const char *str, size_t sz);

aq_obj_t aq_eval(aq_state_t *aq, aq_obj_t obj);

/* constructs a closure object from a form, but doesn't execute it */
aq_obj_t aq_compile_form(aq_state_t *aq, aq_obj_t obj);

void aq_print_closure(aq_state_t *aq, aq_obj_t obj, FILE *file);

void aq_display(aq_state_t *aq, aq_obj_t obj, FILE *file);

/*==========================================================================*/

/* not to be called by users */
void aq_freeze_var(aq_state_t *aq, aq_obj_t *obj);
void aq_unfreeze_var(aq_state_t *aq, aq_obj_t *obj);

#define aq_declare_var(aq, name)                                               \
    aq_obj_t name;                                                             \
    name.t = AQ_OBJ_NIL;                                                       \
    aq_freeze_var(aq, &name)

#define aq_var1(aq, name1) aq_declare_var(aq, name1)
#define aq_var2(aq, name1, name2)                                              \
    aq_var1(aq, name1);                                                        \
    aq_declare_var(aq, name2)
#define aq_var3(aq, name1, name2, name3)                                       \
    aq_var2(aq, name1, name2);                                                 \
    aq_declare_var(aq, name3)
#define aq_var4(aq, name1, name2, name3, name4)                                \
    aq_var3(aq, name1, name2, name3);                                          \
    aq_declare_var(aq, name4)
#define aq_var5(aq, name1, name2, name3, name4, name5)                         \
    aq_var4(aq, name1, name2, name3, name4);                                   \
    aq_declare_var(aq, name5)
#define aq_var6(aq, name1, name2, name3, name4, name5, name6)                  \
    aq_var5(aq, name1, name2, name3, name4, name5);                            \
    aq_declare_var(aq, name6)
#define aq_var7(aq, name1, name2, name3, name4, name5, name6, name7)           \
    aq_var6(aq, name1, name2, name3, name4, name5, name6);                     \
    aq_declare_var(aq, name7)
#define aq_var8(aq, name1, name2, name3, name4, name5, name6, name7, name8)    \
    aq_var7(aq, name1, name2, name3, name4, name5, name6, name7);              \
    aq_declare_var(aq, name8)
#define aq_var9(aq, name1, name2, name3, name4, name5, name6, name7, name8,    \
                name9)                                                         \
    aq_var8(aq, name1, name2, name3, name4, name5, name6, name7, name8);       \
    aq_declare_var(aq, name9)

#define aq_release_var(aq, name) aq_unfreeze_var(aq, &name)

#define aq_release1(aq, name1) aq_release_var(aq, name1)
#define aq_release2(aq, name1, name2)                                          \
    aq_release1(aq, name1);                                                    \
    aq_release_var(aq, name2)
#define aq_release3(aq, name1, name2, name3)                                   \
    aq_release2(aq, name1, name2);                                             \
    aq_release_var(aq, name3)
#define aq_release4(aq, name1, name2, name3, name4)                            \
    aq_release3(aq, name1, name2, name3);                                      \
    aq_release_var(aq, name4)
#define aq_release5(aq, name1, name2, name3, name4, name5)                     \
    aq_release4(aq, name1, name2, name3, name4);                               \
    aq_release_var(aq, name5)
#define aq_release6(aq, name1, name2, name3, name4, name5, name6)              \
    aq_release5(aq, name1, name2, name3, name4, name5);                        \
    aq_release_var(aq, name6)
#define aq_release7(aq, name1, name2, name3, name4, name5, name6, name7)       \
    aq_release6(aq, name1, name2, name3, name4, name5, name6);                 \
    aq_release_var(aq, name7)
#define aq_release8(aq, name1, name2, name3, name4, name5, name6, name7,       \
                    name8)                                                     \
    aq_release7(aq, name1, name2, name3, name4, name5, name6, name7);          \
    aq_release_var(aq, name8)
#define aq_release9(aq, name1, name2, name3, name4, name5, name6, name7,       \
                    name8, name9)                                              \
    aq_release8(aq, name1, name2, name3, name4, name5, name6, name7, name8);   \
    aq_release_var(aq, name9)

#endif
