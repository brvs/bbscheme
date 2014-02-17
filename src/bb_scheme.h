
#ifndef _BB_SCHEME_H
#define _BB_SCHEME_H

#include <stdbool.h>


/********** Model layer **********/

typedef enum {
    BB_FIXNUM, BB_BOOLEAN, BB_CHARACTER, BB_STRING, BB_SYMBOL,
    BB_EMPTY_LIST, BB_PAIR, BB_PRIMITIVE_PROCEDURE, BB_COMPOUND_PROCEDURE,
    BB_VOID,
} bb_object_type;

typedef struct bb_object {
    bb_object_type type;

    union {
        long fixnum;
        bool boolean;
        char character;
        char *string;
        char *symbol;

        struct {
            struct bb_object *car; // any object
            struct bb_object *cdr; // any object
        } pair;

        struct {
            struct bb_object *(*fn)(struct bb_object *args);
            struct bb_object *name; // symbol
        } prim_proc;

        struct {
            struct bb_object *name; // symbol or TODO empty/null
            struct bb_object *formals;
            struct bb_object *body;
            struct bb_object *env;
        } compound_proc;

    } value;

} bb_object;

int obj_count;

bb_object *bb_true, *bb_false, *bb_empty_list, *bb_void, *bb_symbol_table,
    *bb_quote_symbol, *bb_define_symbol, *bb_set_symbol, *bb_ok_symbol,
    *bb_if_symbol, *bb_cond_symbol, *bb_begin_symbol, *bb_lambda_symbol,
    *bb_let_symbol, *bb_void_symbol,
    *bb_empty_environment, *bb_global_environment;

bb_object *bb_alloc_object();
bool bb_isfixnum(bb_object *);
bool bb_isboolean(bb_object *);
bool bb_isfalse(bb_object *);
bool bb_istrue(bb_object *);
bool bb_ischaracter(bb_object *);
bool bb_isstring(bb_object *);
bool bb_isemptylist(bb_object *);
bool bb_ispair(bb_object *);
bool bb_issymbol(bb_object *);
bool bb_isprimitiveproc(bb_object *);
bool bb_iscompoundproc(bb_object *);
bool bb_iscallable(bb_object *);

bb_object *bb_make_fixnum(long);
bb_object *bb_make_bool(bool);
bb_object *bb_make_character(char);
bb_object *bb_make_string(char *);
bb_object *bb_cons(bb_object *car, bb_object *cdr);
void bb_set_car(bb_object *pair, bb_object *value);
void bb_set_cdr(bb_object *pair, bb_object *value);
bb_object *bb_car(bb_object *);
bb_object *bb_cdr(bb_object *);
#define bb_cadr(x) bb_car(bb_cdr(x))
#define bb_caadr(x) bb_car(bb_car(bb_cdr(x)))
#define bb_caddr(x) bb_car(bb_cdr(bb_cdr(x)))
#define bb_cadddr(x) bb_car(bb_cdr(bb_cdr(bb_cdr(x))))
#define bb_caar(x) bb_car(bb_car(x))
#define bb_cdar(x) bb_cdr(bb_car(x))
#define bb_cddr(x) bb_cdr(bb_cdr(x))
bb_object *bb_make_symbol(char *);
bb_object *bb_make_compound_proc(bb_object *name, bb_object *formals,
                              bb_object *body, bb_object *env);

void bb_init();


/********** Read **********/

bb_object *bb_read(FILE*);


/********** Write **********/

void bb_write(bb_object *);

/********** Eval **********/

bb_object *bb_extend_environment(bb_object *vars, bb_object *vals,
                                 bb_object *env);
bb_object *bb_setup_environment();
bb_object *bb_eval(bb_object *, bb_object *);

bb_object *bb_current_frame(bb_object *env);
bb_object *bb_load_file(char *);

/********* Procedures ******/

int bb_list_size(bb_object *list);

// TODO use these throughout
#define BB_ARITY_MISMATCH "Arity mismatch, args expected: "
#define BB_ARGS_GT(args, n) if(bb_list_size(args) <= n) { \
        fprintf(stderr, BB_ARITY_MISMATCH "> %d\n", n);     \
        exit(-1); }                                       
#define BB_ARGS_LT(args, n) if(bb_list_size(args) >= n) { \
        fprintf(stderr, BB_ARITY_MISMATCH "< %d\n", n);     \
        exit(-1); }                                       
#define BB_ARGS_EQ(args, n) if(bb_list_size(args) != n) { \
        fprintf(stderr, BB_ARITY_MISMATCH "%d\n", n);     \
        exit(-1); }                                       

void bb_init_procs(bb_object *env);
void bb_frame_bind_var(bb_object *frame, bb_object *var, bb_object *val);

#endif /* _BB_SCHEME_H */
