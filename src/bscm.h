
#ifndef _SCHEME_H
#define _SCHEME_H

#include <stdbool.h>


/********** Model layer **********/

typedef enum {
    FIXNUM, BOOLEAN, CHARACTER, STRING, SYMBOL,
    EMPTY_LIST, PAIR, PRIMITIVE_PROCEDURE, COMPOUND_PROCEDURE,
    VOID,
} object_type_t;

typedef struct object_t {
    object_type_t type;

    union {
        long fixnum;
        bool boolean;
        char character;
        char *string;
        char *symbol;

        struct {
            struct object_t *car; // any object_t
            struct object_t *cdr; // any object_t
        } pair;

        struct {
            struct object_t *(*fn)(struct object_t *args);
            struct object_t *name; // symbol
        } prim_proc;

        struct {
            struct object_t *name; // symbol or TODO empty/null
            struct object_t *formals;
            struct object_t *body;
            struct object_t *env;
        } compound_proc;

    } value;

} object_t;

int obj_count;

object_t *true_obj, *false_obj, *empty_list, *void_obj, *symbol_table,
    *quote_symbol, *define_symbol, *set_symbol, *ok_symbol,
    *if_symbol, *cond_symbol, *begin_symbol, *lambda_symbol,
    *let_symbol, *void_symbol,
    *empty_environment, *global_environment;

object_t *alloc_object_t();
bool isfixnum(object_t *);
bool isboolean(object_t *);
bool isfalse(object_t *);
bool istrue(object_t *);
bool ischaracter(object_t *);
bool isstring(object_t *);
bool isemptylist(object_t *);
bool ispair(object_t *);
bool issymbol(object_t *);
bool isprimitiveproc(object_t *);
bool iscompoundproc(object_t *);
bool iscallable(object_t *);

object_t *make_fixnum(long);
object_t *make_bool(bool);
object_t *make_character(char);
object_t *make_string(char *);
object_t *cons(object_t *car, object_t *cdr);
void set_car(object_t *pair, object_t *value);
void set_cdr(object_t *pair, object_t *value);
object_t *car(object_t *);
object_t *cdr(object_t *);
#define cadr(x) car(cdr(x))
#define caadr(x) car(car(cdr(x)))
#define caddr(x) car(cdr(cdr(x)))
#define cadddr(x) car(cdr(cdr(cdr(x))))
#define caar(x) car(car(x))
#define cdar(x) cdr(car(x))
#define cddr(x) cdr(cdr(x))
object_t *make_symbol(char *);
object_t *make_compound_proc(object_t *name, object_t *formals,
                              object_t *body, object_t *env);

void init();


/********** Read **********/

object_t *read(FILE*);


/********** Write **********/

void write(object_t *);

/********** Eval **********/

object_t *extend_environment(object_t *vars, object_t *vals,
                                 object_t *env);
object_t *setup_environment();
object_t *eval(object_t *, object_t *);

object_t *current_frame(object_t *env);
object_t *load_file(char *);

/********* Procedures ******/

int list_size(object_t *list);

// TODO use these throughout
#define ARITY_MISMATCH "Arity mismatch, args expected: "
#define ARGS_GT(args, n) if(list_size(args) <= n) { \
        fprintf(stderr, ARITY_MISMATCH "> %d\n", n);     \
        exit(-1); }
#define ARGS_LT(args, n) if(list_size(args) >= n) { \
        fprintf(stderr, ARITY_MISMATCH "< %d\n", n);     \
        exit(-1); }
#define ARGS_EQ(args, n) if(list_size(args) != n) { \
        fprintf(stderr, ARITY_MISMATCH "%d\n", n);     \
        exit(-1); }

void init_procs(object_t *env);
void frame_bind_var(object_t *frame, object_t *var, object_t *val);

#endif /* _SCHEME_H */
