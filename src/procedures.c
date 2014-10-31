
#include <stdio.h>
#include <stdlib.h>

#include "bscm.h"

// TODO return nothing if necessary

/*
 * Utility functions
 */


int list_size(object_t *list) {
    int i = 0;
    while(!isemptylist(list)) {
        i++;
        list = cdr(list);
    }
    return i;
}


/*
 * Scheme functions
 */

static object_t *make_primitive_procedure(object_t *(*fn)(object_t*),
                                              object_t *name) {
    object_t *proc = alloc_object_t();
    proc->type = PRIMITIVE_PROCEDURE;
    proc->value.prim_proc.fn = fn;
    proc->value.prim_proc.name = name;
    return proc;
}

static object_t *object_type_proc(object_t *args) {
    ARGS_EQ(args, 1);
    return make_fixnum(car(args)->type);
}

static object_t *add_proc(object_t *args) {
    int sum = 0;
    while(!isemptylist(args)) {
        sum += car(args)->value.fixnum;
        args = cdr(args);
    }
    return make_fixnum(sum);
}

static object_t *multiply_proc(object_t *args) {
    int product = 1;
    while(!isemptylist(args)) {
        product *= car(args)->value.fixnum;
        args = cdr(args);
    }
    return make_fixnum(product);
}

static object_t *subtract_proc(object_t *args) {
    if(list_size(args) < 1) {
        fprintf(stderr, "Arity mismatch\n");
        exit(-1);
    }
    int difference = car(args)->value.fixnum;
    args = cdr(args);
    while(!isemptylist(args)) {
        difference -= car(args)->value.fixnum;
        args = cdr(args);
    }
    return make_fixnum(difference);
}

static object_t *divide_proc(object_t *args) {
    int args_count = list_size(args);
    if(args_count < 1) {
        fprintf(stderr, "Arity mismatch\n");
        exit(-1);
    } else if(args_count == 1) {
        return make_fixnum(1 / car(args)->value.fixnum);
    } else {
        int quotient = car(args)->value.fixnum;
        args = cdr(args);
        while(!isemptylist(args)) {
            quotient /= car(args)->value.fixnum;
            args = cdr(args);
        }
        return make_fixnum(quotient);
    }
}

static object_t *puts_proc(object_t *args) {
    ARGS_EQ(args, 1);
    object_t *obj = car(args);
    if(isstring(obj)) {
        printf("%s", obj->value.string);
    } // TODO else
    return void_obj;
}

static object_t *newline_proc(object_t *args) {
    ARGS_EQ(args, 0);
    printf("\n");
    return void_obj;
}

static object_t *load_proc(object_t *args) {
    ARGS_EQ(args, 1);
    return load_file( car(args)->value.string );
}

static object_t *cons_proc(object_t *args) {
    ARGS_EQ(args, 2);
    return cons(car(args), cadr(args));
}

static object_t *car_proc(object_t *args) {
    ARGS_EQ(args, 1);
    return caar(args);
}

static object_t *cdr_proc(object_t *args) {
    ARGS_EQ(args, 1);
    return cdar(args);
}

static object_t *list_proc(object_t *args) {
    return args;
}

static object_t *void_proc(object_t *args) {
    return void_obj;
}

static object_t *and_proc(object_t *args) {
    bool result = true;
    while(!isemptylist(args)) {
        if(isfalse(car(args))) {
            result = 0;
            break;
        }
        args = cdr(args);
    }
    return make_bool(result);
}

static object_t *or_proc(object_t *args) {
    bool result = false;
    while(!isemptylist(args)) {
        if(istrue(car(args))) {
            result = 1;
            break;
        }
        args = cdr(args);
    }
    return make_bool(result);
}

/* #define MAKE_COMPARISON(NAME, COMP)                                 \ */
/*     static object_t * ##NAME##_proc(object_t *args) {                \ */
/*         ARGS_GT(args, 1);                                           \ */
/*         object_t *prev = car(args);                                 \ */
/*         args = cdr(args);                                           \ */
/*         char res = 1;                                               \ */
/*         while(!isemptylist(args)) {                                 \ */
/*             object_t *this = car(args);                             \ */
/*             if(! (prev->value.fixnum COMP this->value.fixnum) ) {   \ */
/*                 res = 0;                                            \ */
/*                 break;                                              \ */
/*             }                                                       \ */
/*             prev = this;                                            \ */
/*             args = cdr(args);                                       \ */
/*         }                                                           \ */
/*         return make_bool(res);                                      \ */
/*     } */

/* MAKE_COMPARISON(e, ==); */
/* MAKE_COMPARISON(gt, >); */
/* MAKE_COMPARISON(gte, >=); */
/* MAKE_COMPARISON(lt, <); */
/* MAKE_COMPARISON(lte, <=); */

static object_t *number_to_string_proc(object_t *args) {
    ARGS_EQ(args, 1);
    char buff[128] = {0};
    sprintf(buff, "%ld", car(args)->value.fixnum);
    return make_string(buff);
}

static object_t *display_proc(object_t *args) {
    ARGS_EQ(args, 1);
    write(car(args));
    return void_obj;
}

static void bind_proc(char *sym, object_t *(*fn)(object_t*),
                         object_t *env) {
    object_t *symbol = make_symbol(sym);
    frame_bind_var(current_frame(env), symbol,
                      make_primitive_procedure(fn, symbol));
}

/*
 * Exported globals
 */

void init_procs(object_t *env) {
    bind_proc("%bb-object-type", object_type_proc, env);
    bind_proc("%bb-puts", puts_proc, env);

    bind_proc("+", add_proc, env);
    bind_proc("-", subtract_proc, env);
    bind_proc("*", multiply_proc, env);
    bind_proc("/", divide_proc, env);
    bind_proc("newline", newline_proc, env);
    bind_proc("load", load_proc, env);

    bind_proc("cons", cons_proc, env);
    bind_proc("car", car_proc, env);
    bind_proc("cdr", cdr_proc, env);
    bind_proc("list", list_proc, env);

    /* bind_proc("=", e_proc, env); */
    /* bind_proc(">", gt_proc, env); */
    /* bind_proc(">=", gte_proc, env); */
    /* bind_proc("<", lt_proc, env); */
    /* bind_proc("<=", lte_proc, env); */

    bind_proc("or", or_proc, env);
    bind_proc("and", and_proc, env);

    bind_proc("number->string", number_to_string_proc, env);

    bind_proc("display", display_proc, env);
    bind_proc("void", void_proc, env);
}
