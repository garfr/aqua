#include <stdlib.h>
#include <stdio.h>

#include "gc.h"

#define HEAP_ALIGN 32

#define ALIGN(num) ((num) + HEAP_ALIGN - ((num) % HEAP_ALIGN))

void *aq_gc_alloc(aq_state_t *aq, size_t amt, uint8_t bit) {
    aq_heap_obj_t *ret = aq->alloc(NULL, 0, amt);
    if (ret == NULL) {
        aq_collect_garbage(aq);
        ret = aq->alloc(NULL, 0, amt);
        if (ret == NULL) {
            aq_panic(aq, AQ_ERR_OOM);
        }
    }

    ret->bit = bit;
    ret->gc_forward = aq->gc_root;
    aq->gc_root = ret;
    return (void *)ret;
}

/* this is a simple non-incremental garbage collector. it simply traverses the
 * roots, marks used data and then sweeps through the gc_list freeing the rest
 */
/* the roots are the program registers, the global table, and any reserved
 * objects */

static void mark_template(aq_template_t *t) {
    t->mark = 1;
    return;
}

static void mark_obj(aq_obj_t obj) {
    aq_pair_t *pair;
    aq_tbl_t *tbl;
    aq_closure_t *clos;
    if (OBJ_IS_HEAP_ANY(obj)) {
        aq_heap_obj_t *hp = obj.v.h;
        if (hp->mark == 1) {
            return;
        }
        hp->mark = 1;
        switch (obj.t) {
        case AQ_OBJ_PAIR:
            pair = (aq_pair_t *)hp;
            mark_obj(pair->car);
            mark_obj(pair->cdr);
            break;
        case AQ_OBJ_TABLE: {
            tbl = (aq_tbl_t *)hp;
            for (size_t i = 0; i < tbl->buckets_sz; i++) {
                for (aq_tbl_entry_t *iter = tbl->buckets[i]; iter != NULL;
                     iter = iter->n) {
                    mark_obj(iter->k);
                    mark_obj(iter->v);
                }
            }
            break;
        case AQ_OBJ_CLOSURE:
            clos = (aq_closure_t *)hp;
            mark_template(clos->t);
            break;
        case AQ_OBJ_CONTIN:
        case AQ_OBJ_BIGNUM:
        case AQ_OBJ_ARRAY:
        case AQ_OBJ_SYM:
        case AQ_OBJ_NIL:
        case AQ_OBJ_CHAR:
        case AQ_OBJ_NUM:
        case AQ_OBJ_TRUE:
        case AQ_OBJ_FALSE:
            break;
        }
        }
    } else {
        return;
    }
}

void free_template(aq_state_t *aq, aq_template_t *t) {
    aq->alloc((void *)t->name, 0, 0);
    aq->alloc((void *)t->lits, 0, 0);
    aq->alloc((void *)t->code, 0, 0);
}

void free_table(aq_state_t *aq, aq_tbl_t *tbl) {
    for (size_t i = 0; i < tbl->buckets_sz; i++) {
        aq_tbl_entry_t *tbl_follow, *tbl_first;
        for (tbl_first = tbl->buckets[i]; tbl_first != NULL;) {
            tbl_follow = tbl_first;
            tbl_first = tbl_first->n;
            aq->alloc(tbl_follow, 0, 0);
        }
    }
    aq->alloc(tbl->buckets, 0, 0);
}

void dispense_obj(aq_state_t *aq, aq_heap_obj_t *obj) {
    switch (obj->bit) {
    case HEAP_TEMPLATE:
        free_template(aq, (aq_template_t *)obj);
        break;
    case HEAP_TABLE:
        free_table(aq, (aq_tbl_t *)obj);
        break;
    default:
        break;
    }
    aq->alloc(obj, 0, 0);
}

void aq_collect_garbage(aq_state_t *aq) {
    /* start with the variable stack */
    for (size_t i = 0; i < aq->vars_sz; i++) {
        mark_obj(aq->vars[i]);
    }

    /* mark all of the frozen variables */
    for (size_t i = 0; i < aq->frozen_sz; i++) {
        if (aq->frozen[i] != NULL) {
            mark_obj(*aq->frozen[i]);
        }
    }

    /* mark the global table */
    mark_obj(aq->g);

    aq_heap_obj_t *first, *follow, *temp;
    first = aq->gc_root;
    follow = NULL;

    while (first != NULL) {
        /* for now don't free symbols */
        if (first->mark == 0 && first->bit != HEAP_SYM) {
            if (follow != NULL) {
                follow->gc_forward = first->gc_forward;
                temp = first;
                first = first->gc_forward;
                dispense_obj(aq, temp);
            } else {
                aq->gc_root = first->gc_forward;
                temp = first;
                first = first->gc_forward;
                dispense_obj(aq, temp);
            }
        } else {
            follow = first;
            first = first->gc_forward;
        }
    }

    for (first = aq->gc_root; first != NULL; first = first->gc_forward) {
        first->mark = 0;
    }
}

void aq_free_all(aq_state_t *aq) {
    aq_heap_obj_t *first, *follow;
    for (first = aq->gc_root; first != NULL;) {
        follow = first;
        first = first->gc_forward;
        switch (follow->bit) {
        case HEAP_TEMPLATE:
            free_template(aq, (aq_template_t *)follow);
            break;
        case HEAP_TABLE:
            free_table(aq, (aq_tbl_t *)follow);
            break;
        default:
            break;
        }
        aq->alloc(follow, 0, 0);
    }
}
