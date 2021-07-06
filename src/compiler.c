#include <ctype.h>
#include <stdarg.h>

#include "compiler.h"
#include "gc.h"
#include "string.h"
#include "helpers.h"

#define INIT_CODE_CAP 8
#define INIT_LITS_CAP 8

typedef struct {
    size_t reg;
    aq_sym_t *sym;
} lex_var;

typedef struct lex_state {
    struct lex_env *up;
} lex_state;

/* the state needed to compile one function, these are created recursively and
 * the final state of it after compilation is used to construct templates and
 * then a simple closure */
typedef struct {
    uint32_t *code;
    size_t code_sz;
    size_t code_cap;

    size_t next_reg;

    aq_obj_t *lits;
    size_t lits_sz;
    size_t lits_cap;

    lex_state *lex;
} comp_fn;

/* after the code to generate a value is created, this object is passed to point
 * where the object will be */
typedef struct {
    enum {
        LOC_REG,
        LOC_LIT,
        LOC_UNIT,
    } t;
    size_t idx;
} val_loc;

typedef enum { ARITH_ADD, ARITH_SUB, ARITH_MUL, ARITH_DIV } arith_op;

typedef enum {
    TOK_OPEN_PAREN,
    TOK_CLOSE_PAREN,
    TOK_NUM,
    TOK_SYM,
    TOK_EOF
} token_kind;

typedef struct {
    size_t l;
    token_kind t;
    union {
        aq_sym_t *sym;
        double num;
    } v;
} token;

typedef struct {
    bool needs_free;
    const char *b;
    size_t sz;
    size_t l;
    size_t s_pos;
    size_t e_pos;

    bool peekf;
    token peek;
} reader;

static const char *sym_chars = "!$%&*+-./:<=>?@^_~";

#define NEXT_C(reader) (reader->b[reader->e_pos++])
#define PEEK_C(reader) (reader->b[reader->e_pos])
#define SKIP_C(reader) (reader->e_pos++)
#define IS_EOF(reader) (reader->e_pos >= reader->sz)
#define RESET(reader) (reader->s_pos = reader->e_pos)

void add_inst(aq_state_t *aq, comp_fn *fn, uint32_t inst) {
    if (fn->code_sz + 1 >= fn->code_cap) {
        fn->code = aq->alloc(fn->code, fn->code_cap * sizeof(uint32_t),
                             fn->code_cap * sizeof(uint32_t) * 2);
        fn->code_cap *= 2;
    }
    fn->code[fn->code_sz++] = inst;
}

int vasprintf(aq_state_t *aq, char **str, const char *fmt, va_list args) {
    int size = 0;
    va_list tmpa;
    va_copy(tmpa, args);
    size = vsnprintf(NULL, 0, fmt, tmpa);
    va_end(tmpa);
    if (size < 0)
        return -1;
    *str = aq->alloc(NULL, 0, size + 1);
    if (NULL == *str)
        return -1;
    size = vsprintf(*str, fmt, args);
    return size;
}

int asprintf(aq_state_t *aq, char **str, const char *fmt, ...) {
    int size = 0;
    va_list args;
    va_start(args, fmt);
    size = vasprintf(aq, str, fmt, args);
    va_end(args);
    return size;
}

void __attribute__((noreturn))
log_err(aq_state_t *aq, size_t line, const char *str) {
    char *msg;
    if (line != 0)
        asprintf(aq, &msg, "%d: %s", line, str);
    else
        asprintf(aq, &msg, "%s", str);

    aq->compiler_err = msg;
    aq_panic(aq, AQ_ERR_SYNTAX);
    exit(EXIT_FAILURE);
}

/* returns false if its in the global enviroment */
bool find_in_scope(comp_fn *fn, aq_sym_t *sym, size_t *idx_out) {
    (void)fn;
    (void)sym;
    (void)idx_out;
    return false;
}

val_loc add_lit(aq_state_t *aq, comp_fn *fn, aq_obj_t obj) {
    if (fn->lits_sz + 1 >= fn->lits_cap) {
        fn->lits = aq->alloc(fn->lits, fn->lits_cap * sizeof(aq_obj_t),
                             fn->lits_cap * sizeof(aq_obj_t) * 2);
        fn->lits_cap *= 2;
    }
    fn->lits[fn->lits_sz] = obj;

    val_loc ret;
    ret.t = LOC_LIT;
    ret.idx = fn->lits_sz;
    fn->lits_sz++;
    return ret;
}

static const uint8_t arith_op_lookup[4][2][2] = {
    [ARITH_ADD] =
        {
            [LOC_REG] =
                {
                    [LOC_REG] = AQ_OP_ADDRR,
                    [LOC_LIT] = AQ_OP_ADDRK,
                },
            [LOC_LIT] =
                {
                    [LOC_REG] = AQ_OP_ADDKR,
                    [LOC_LIT] = AQ_OP_ADDKK,
                },
        },
    [ARITH_SUB] =
        {
            [LOC_REG] =
                {
                    [LOC_REG] = AQ_OP_SUBRR,
                    [LOC_LIT] = AQ_OP_SUBRK,
                },
            [LOC_LIT] =
                {
                    [LOC_REG] = AQ_OP_SUBKR,
                    [LOC_LIT] = AQ_OP_SUBKK,
                },
        },
    [ARITH_MUL] =
        {
            [LOC_REG] =
                {
                    [LOC_REG] = AQ_OP_MULRR,
                    [LOC_LIT] = AQ_OP_MULRK,
                },
            [LOC_LIT] =
                {
                    [LOC_REG] = AQ_OP_MULKR,
                    [LOC_LIT] = AQ_OP_MULKK,
                },
        },
    [ARITH_DIV] =
        {
            [LOC_REG] =
                {
                    [LOC_REG] = AQ_OP_DIVRR,
                    [LOC_LIT] = AQ_OP_DIVRK,
                },
            [LOC_LIT] =
                {
                    [LOC_REG] = AQ_OP_DIVKR,
                    [LOC_LIT] = AQ_OP_DIVKK,
                },
        },
};

val_loc generate_form(aq_state_t *aq, aq_obj_t form, comp_fn *fn);

val_loc generate_display(aq_state_t *aq, aq_obj_t args, comp_fn *fn) {
    if (args.t != AQ_OBJ_PAIR) {
        log_err(aq, 0, "builtin function 'display' takes 1 argument");
    }

    val_loc loc1 = generate_form(aq, OBJ_GET_CAR(args), fn);
    add_inst(aq, fn,
             ENCODE_AD(loc1.t == LOC_LIT ? AQ_OP_DISPLAYK : AQ_OP_DISPLAYR, 0,
                       loc1.idx));

    val_loc ret = {LOC_UNIT, 0};
    return ret;
}

static uint8_t cons_op_lookup[2][2] = {
    [LOC_REG] = {[LOC_REG] = AQ_OP_CONSRR, [LOC_LIT] = AQ_OP_CONSRK},
    [LOC_LIT] = {[LOC_REG] = AQ_OP_CONSKR, [LOC_LIT] = AQ_OP_CONSKK},
};

val_loc generate_define(aq_state_t *aq, aq_obj_t args, comp_fn *fn) {
    if (fn->lex == NULL) {
        if (args.t == AQ_OBJ_PAIR && OBJ_GET_CDR(args).t == AQ_OBJ_PAIR &&
            OBJ_GET_CAR(args).t == AQ_OBJ_SYM) {
            aq_obj_t sym = OBJ_GET_CAR(args);
            val_loc loc1 =
                generate_form(aq, OBJ_GET_CAR(OBJ_GET_CDR(args)), fn);
            size_t sym_idx = add_lit(aq, fn, sym).idx;
            add_inst(aq, fn,
                     ENCODE_AD(loc1.t == LOC_REG ? AQ_OP_GSETKR : AQ_OP_GSETKK,
                               sym_idx, loc1.idx));
            val_loc ret = {LOC_UNIT, 0};
            return ret;
        } else {
            log_err(
                aq, 0,
                "builtin function 'define' takes a symbol and an expression");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("internal: cannot use lexical enviroments yet\n");
        exit(EXIT_FAILURE);
    }
}

val_loc generate_cons(aq_state_t *aq, aq_obj_t args, comp_fn *fn) {
    if (args.t == AQ_OBJ_PAIR && OBJ_GET_CDR(args).t == AQ_OBJ_PAIR) {
        val_loc loc1 = generate_form(aq, OBJ_GET_CAR(args), fn);
        val_loc loc2 = generate_form(aq, OBJ_GET_CAR(OBJ_GET_CDR(args)), fn);
        val_loc ret = {LOC_REG, fn->next_reg++};
        add_inst(aq, fn,
                 ENCODE_ABC(cons_op_lookup[loc1.t][loc2.t], ret.idx, loc1.idx,
                            loc2.idx));
        return ret;
    }
    log_err(aq, 0, "builtin function 'cons' takes 2 arguments");
}

val_loc generate_carcdr(aq_state_t *aq, aq_obj_t args, int car, comp_fn *fn) {
    if (args.t != AQ_OBJ_PAIR) {
        log_err(aq, 0, "builtin functions car and cdr take 1 argument");
    }

    val_loc loc1 = generate_form(aq, OBJ_GET_CAR(args), fn);
    if (loc1.t != LOC_REG) {
        log_err(aq, 0, "cannot take car/cdr of non-pair object");
    }

    val_loc ret = {LOC_REG, fn->next_reg++};

    add_inst(aq, fn, ENCODE_AD(car ? AQ_OP_CAR : AQ_OP_CDR, ret.idx, loc1.idx));
    return ret;
}

val_loc generate_arith(aq_state_t *aq, arith_op op, aq_obj_t args,
                       comp_fn *fn) {
    if (args.t != AQ_OBJ_PAIR && OBJ_GET_CDR(args).t != AQ_OBJ_PAIR) {
        log_err(aq, 0, "arithmetic ops take 2 arguments");
        exit(EXIT_FAILURE);
    }
    val_loc loc1 = generate_form(aq, OBJ_GET_CAR(args), fn);
    val_loc loc2 = generate_form(aq, OBJ_GET_CAR(OBJ_GET_CDR(args)), fn);

    val_loc ret = {LOC_REG, fn->next_reg++};

    add_inst(aq, fn,
             ENCODE_ABC(arith_op_lookup[op][loc1.t][loc2.t], ret.idx, loc1.idx,
                        loc2.idx));
    return ret;
}

val_loc generate_funcall(aq_state_t *aq, aq_obj_t head, aq_obj_t args,
                         comp_fn *fn) {
    if (head.t == AQ_OBJ_SYM) {
        aq_sym_t *sym = OBJ_DECODE_SYM(head);
        if (streq(sym->s, "+", sym->l, 1))
            return generate_arith(aq, ARITH_ADD, args, fn);
        if (streq(sym->s, "-", sym->l, 1))
            return generate_arith(aq, ARITH_SUB, args, fn);
        if (streq(sym->s, "*", sym->l, 1))
            return generate_arith(aq, ARITH_MUL, args, fn);
        if (streq(sym->s, "/", sym->l, 1))
            return generate_arith(aq, ARITH_DIV, args, fn);
        if (streq(sym->s, "display", sym->l, 7))
            return generate_display(aq, args, fn);
        if (streq(sym->s, "cons", sym->l, 4))
            return generate_cons(aq, args, fn);
        if (streq(sym->s, "car", sym->l, 3))
            return generate_carcdr(aq, args, 1, fn);
        if (streq(sym->s, "cdr", sym->l, 3))
            return generate_carcdr(aq, args, 0, fn);
        if (streq(sym->s, "define", sym->l, 6))
            return generate_define(aq, args, fn);
    }
    printf("cannot compile real functions yet\n");
    exit(EXIT_FAILURE);
}

val_loc generate_form(aq_state_t *aq, aq_obj_t form, comp_fn *fn) {
    switch (form.t) {
    case AQ_OBJ_NUM:
        return add_lit(aq, fn, form);
    case AQ_OBJ_PAIR:
        return generate_funcall(aq, OBJ_GET_CAR(form), OBJ_GET_CDR(form), fn);
    case AQ_OBJ_SYM: {
        size_t idx;
        aq_sym_t *s = OBJ_DECODE_HEAP(form, aq_sym_t);
        if (find_in_scope(fn, s, &idx)) {
            printf("cannot use local variables yet");
            exit(EXIT_FAILURE);
        } else {
            val_loc ret = {LOC_REG, fn->next_reg++};
            size_t idx = add_lit(aq, fn, form).idx;
            add_inst(aq, fn, ENCODE_AD(AQ_OP_GGETK, ret.idx, idx));
            return ret;
        }
    }

    default:
        printf("%d\n", form.t);
        log_err(aq, 0, "expected valid form in expression");
    }
    aq_obj_t lit;
    OBJ_ENCODE_NUM(lit, 3.14159);
    return add_lit(aq, fn, lit);
}

comp_fn init_fn(aq_state_t *aq) {
    comp_fn ret;

    ret.code_sz = 0;
    ret.code_cap = INIT_CODE_CAP;
    ret.code = aq->alloc(NULL, 0, sizeof(uint32_t) * ret.code_cap);

    ret.lits_sz = 0;
    ret.lits_cap = INIT_LITS_CAP;
    ret.lits = aq->alloc(NULL, 0, sizeof(aq_obj_t) * ret.lits_cap);

    ret.next_reg = 0;
    ret.lex = NULL;

    return ret;
}

void deinit_fn(aq_state_t *aq, comp_fn *fn) {
    aq->alloc(fn->code, 0, 0);
    aq->alloc(fn->lits, 0, 0);
    fn->code = NULL;
    fn->lits = NULL;
}

aq_closure_t *compile_form(aq_state_t *aq, aq_obj_t form) {
    comp_fn global_fn = init_fn(aq);

    val_loc val = generate_form(aq, form, &global_fn);
    if (val.t != LOC_UNIT) {
        add_inst(
            aq, &global_fn,
            ENCODE_AD(val.t == LOC_LIT ? AQ_OP_RETK : AQ_OP_RETR, 0, val.idx));
    } else {
        add_inst(aq, &global_fn, ENCODE_ABC(AQ_OP_RETNIL, 0, 0, 0));
    }

    aq_template_t *t = GC_NEW(aq, aq_template_t, HEAP_TEMPLATE);

    uint32_t *code = aq->alloc(NULL, 0, sizeof(uint32_t) * global_fn.code_sz);
    memcpy(code, global_fn.code, sizeof(uint32_t) * global_fn.code_sz);
    t->code_sz = global_fn.code_sz;
    t->code = code;

    aq_obj_t *lits = aq->alloc(NULL, 0, sizeof(aq_obj_t) * global_fn.lits_sz);
    memcpy(lits, global_fn.lits, sizeof(aq_obj_t) * global_fn.lits_sz);
    t->lits_sz = global_fn.lits_sz;
    t->lits = lits;

    aq_closure_t *c = GC_NEW(aq, aq_closure_t, HEAP_CLOSURE);
    c->t = t;

    deinit_fn(aq, &global_fn);
    return c;
}

static void skip_whitespace(reader *rd) {
    int c;
    while (!IS_EOF(rd) && isspace(c = PEEK_C(rd))) {
        if (c == '\n') {
            rd->l++;
        }
        rd->e_pos++;
    }
    RESET(rd);
}

static inline token make_tok_inplace(reader *rd, token_kind t) {
    token ret;
    ret.t = t;
    ret.l = rd->l;
    SKIP_C(rd);
    RESET(rd);
    return ret;
}

static inline token make_tok_behind(reader *rd, token_kind t) {
    token ret;
    ret.t = t;
    ret.l = rd->l;
    RESET(rd);
    return ret;
}

static token lex_num(aq_state_t *aq, reader *rd) {
    (void)aq;
    double num = 0;
    while (!IS_EOF(rd) && isdigit(PEEK_C(rd)))
        SKIP_C(rd);

    if (!IS_EOF(rd) && PEEK_C(rd) == '.') {
        SKIP_C(rd);
        while (!IS_EOF(rd) && isdigit(PEEK_C(rd)))
            SKIP_C(rd);
    }

    num = strtod(rd->b + rd->s_pos, NULL);

    token ret = make_tok_behind(rd, TOK_NUM);
    ret.v.num = num;
    return ret;
}

static token lex_sym(aq_state_t *aq, reader *rd) {
    int c;
    while (!IS_EOF(rd) &&
           (isalpha(c = PEEK_C(rd)) || strchr(sym_chars, c) != NULL)) {
        SKIP_C(rd);
    }
    aq_sym_t *sym = aq_intern_sym(aq, &rd->b[rd->s_pos], rd->e_pos - rd->s_pos);
    token ret = make_tok_behind(rd, TOK_SYM);
    ret.v.sym = sym;
    return ret;
}

static token get_token(aq_state_t *aq, reader *rd) {
    skip_whitespace(rd);

    if (IS_EOF(rd)) {
        return make_tok_inplace(rd, TOK_EOF);
    }
    int c = PEEK_C(rd);
    switch (c) {
    case '(':
        return make_tok_inplace(rd, TOK_OPEN_PAREN);
    case ')':
        return make_tok_inplace(rd, TOK_CLOSE_PAREN);
    default:
        break;
    }
    if (isalpha(c) || strchr(sym_chars, c) != NULL) {
        return lex_sym(aq, rd);
    }
    if (isdigit(c)) {
        return lex_num(aq, rd);
    }
    log_err(aq, rd->l, "invalid character");
    return make_tok_inplace(rd, TOK_EOF);
}

static token peek_token(aq_state_t *aq, reader *rd) {
    if (rd->peekf) {
        return rd->peek;
    }
    rd->peekf = true;
    rd->peek = get_token(aq, rd);
    return rd->peek;
}

static token next_token(aq_state_t *aq, reader *rd) {
    if (rd->peekf) {
        rd->peekf = false;
        return rd->peek;
    }
    return get_token(aq, rd);
}

static bool read_expr(aq_state_t *aq, reader *rd, aq_obj_t *out);

static aq_obj_t read_list(aq_state_t *aq, reader *rd) {
    aq_var4(aq, list, temp, rev1, rev2);

    OBJ_ENCODE_NIL(list);

    while (peek_token(aq, rd).t != TOK_CLOSE_PAREN) {
        read_expr(aq, rd, &temp);
        list = aq_create_pair(aq, temp, list);
    }
    next_token(aq, rd);

    OBJ_ENCODE_NIL(rev1);
    while (list.t != AQ_OBJ_NIL) {
        rev2 = list;
        list = OBJ_GET_CDR(list);
        aq_pair_t *pair = (aq_pair_t *)rev2.v.h;
        pair->cdr = rev1;
        rev1 = rev2;
    }

    aq_release4(aq, list, temp, rev1, rev2);
    return rev1;
}

static bool read_expr(aq_state_t *aq, reader *rd, aq_obj_t *out) {
    token tok = next_token(aq, rd);
    switch (tok.t) {
    case TOK_SYM:
        OBJ_ENCODE_SYM((*out), tok.v.sym);
        return true;
    case TOK_NUM:
        OBJ_ENCODE_NUM((*out), tok.v.num);
        return true;
    case TOK_OPEN_PAREN:
        *out = read_list(aq, rd);
        return true;
    case TOK_EOF:
        return false;
    default:
        log_err(aq, tok.l, "expected expression");
        OBJ_ENCODE_NIL((*out));
        return false;
    }
}

aq_obj_t aq_eval_string(aq_state_t *aq, const char *str, size_t sz) {
    reader rd;
    rd.needs_free = false;
    rd.peekf = false;
    rd.b = str;
    rd.l = 1;
    rd.sz = sz;
    rd.s_pos = rd.e_pos = 0;

    aq_obj_t ret;
    read_expr(aq, &rd, &ret);
    return ret;
}
aq_obj_t aq_eval_file(aq_state_t *aq, const char *filename) {
    reader rd;
    rd.needs_free = true;
    rd.peekf = false;

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        aq_panic(aq, AQ_ERR_INVALID_FILENAME);
    }

    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    rewind(f);

    char *str = aq->alloc(NULL, 0, sizeof(char) * len);
    fread(str, 1, len, f);
    fclose(f);

    rd.b = str;
    rd.sz = len;
    rd.l = 1;
    rd.s_pos = rd.e_pos = 0;

    aq_var2(aq, form, res);

    while (read_expr(aq, &rd, &form)) {
        res = aq_eval(aq, form);
    }

    aq->alloc(str, 0, 0);
    return res;
}
