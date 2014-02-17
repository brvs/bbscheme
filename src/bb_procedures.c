
#include <stdio.h>
#include <stdlib.h>

#include "bb_scheme.h"

// TODO return nothing if necessary

/*
 * Utility functions
 */


int bb_list_size(bb_object *list) {
    int i = 0;
    while(!bb_isemptylist(list)) {
        i++;
        list = bb_cdr(list);
    }
    return i;
}


/*
 * Scheme functions
 */

static bb_object *bb_make_primitive_procedure(bb_object *(*fn)(bb_object*),
                                              bb_object *name) {
    bb_object *proc = bb_alloc_object();
    proc->type = BB_PRIMITIVE_PROCEDURE;
    proc->value.prim_proc.fn = fn;
    proc->value.prim_proc.name = name;
    return proc;
}

static bb_object *bb_object_type_proc(bb_object *args) {
    BB_ARGS_EQ(args, 1);
    return bb_make_fixnum(bb_car(args)->type);
}

static bb_object *bb_add_proc(bb_object *args) {
    int sum = 0;
    while(!bb_isemptylist(args)) {
        sum += bb_car(args)->value.fixnum;
        args = bb_cdr(args);
    }
    return bb_make_fixnum(sum);
}

static bb_object *bb_multiply_proc(bb_object *args) {
    int product = 1;
    while(!bb_isemptylist(args)) {
        product *= bb_car(args)->value.fixnum;
        args = bb_cdr(args);
    }
    return bb_make_fixnum(product);
}

static bb_object *bb_subtract_proc(bb_object *args) {
    if(bb_list_size(args) < 1) {
        fprintf(stderr, "Arity mismatch\n");
        exit(-1);
    }
    int difference = bb_car(args)->value.fixnum;
    args = bb_cdr(args);
    while(!bb_isemptylist(args)) {
        difference -= bb_car(args)->value.fixnum;
        args = bb_cdr(args);
    }
    return bb_make_fixnum(difference);
}

static bb_object *bb_divide_proc(bb_object *args) {
    int args_count = bb_list_size(args);
    if(args_count < 1) {
        fprintf(stderr, "Arity mismatch\n");
        exit(-1);
    } else if(args_count == 1) {
        return bb_make_fixnum(1 / bb_car(args)->value.fixnum);
    } else {
        int quotient = bb_car(args)->value.fixnum;
        args = bb_cdr(args);
        while(!bb_isemptylist(args)) {
            quotient /= bb_car(args)->value.fixnum;
            args = bb_cdr(args);
        }
        return bb_make_fixnum(quotient);
    }
}

static bb_object *bb_puts_proc(bb_object *args) {
    BB_ARGS_EQ(args, 1);
    bb_object *obj = bb_car(args);
    if(bb_isstring(obj)) {
        printf("%s", obj->value.string);
    } // TODO else
    return bb_void;
}

static bb_object *bb_newline_proc(bb_object *args) {
    BB_ARGS_EQ(args, 0);
    printf("\n");
    return bb_void;
}

static bb_object *bb_load_proc(bb_object *args) {
    BB_ARGS_EQ(args, 1);
    return bb_load_file( bb_car(args)->value.string );
}

static bb_object *bb_cons_proc(bb_object *args) {
    BB_ARGS_EQ(args, 2);
    return bb_cons(bb_car(args), bb_cadr(args));
}

static bb_object *bb_car_proc(bb_object *args) {
    BB_ARGS_EQ(args, 1);
    return bb_caar(args);
}

static bb_object *bb_cdr_proc(bb_object *args) {
    BB_ARGS_EQ(args, 1);
    return bb_cdar(args);
}

static bb_object *bb_list_proc(bb_object *args) {
    return args;
}

static bb_object *bb_void_proc(bb_object *args) {
    return bb_void;
}

static bb_object *bb_and_proc(bb_object *args) {
    bool result = true;
    while(!bb_isemptylist(args)) {
        if(bb_isfalse(bb_car(args))) {
            result = 0;
            break;
        }
        args = bb_cdr(args);
    }
    return bb_make_bool(result);
}

static bb_object *bb_or_proc(bb_object *args) {
    bool result = false;
    while(!bb_isemptylist(args)) {
        if(bb_istrue(bb_car(args))) {
            result = 1;
            break;
        }
        args = bb_cdr(args);
    }
    return bb_make_bool(result);
}

#define BB_MAKE_COMPARISON(NAME, COMP)                              \
    static bb_object *bb_##NAME##_proc(bb_object *args) {           \
        BB_ARGS_GT(args, 1);                                        \
        bb_object *prev = bb_car(args);                             \
        args = bb_cdr(args);                                        \
        char res = 1;                                               \
        while(!bb_isemptylist(args)) {                              \
            bb_object *this = bb_car(args);                         \
            if(! (prev->value.fixnum COMP this->value.fixnum) ) {   \
                res = 0;                                            \
                break;                                              \
            }                                                       \
            prev = this;                                            \
            args = bb_cdr(args);                                    \
        }                                                           \
        return bb_make_bool(res);                                   \
    }

BB_MAKE_COMPARISON(e, ==);
BB_MAKE_COMPARISON(gt, >);
BB_MAKE_COMPARISON(gte, >=);
BB_MAKE_COMPARISON(lt, <);
BB_MAKE_COMPARISON(lte, <=);

static bb_object *bb_number_to_string_proc(bb_object *args) {
    BB_ARGS_EQ(args, 1);
    char buff[128] = {0};
    sprintf(buff, "%ld", bb_car(args)->value.fixnum);
    return bb_make_string(buff);
}

static bb_object *bb_display_proc(bb_object *args) {
    BB_ARGS_EQ(args, 1);
    bb_write(bb_car(args));
    return bb_void;
}

static void bb_bind_proc(char *sym, bb_object *(*fn)(bb_object*),
                         bb_object *env) {
    bb_object *symbol = bb_make_symbol(sym);
    bb_frame_bind_var(bb_current_frame(env), symbol,
                      bb_make_primitive_procedure(fn, symbol));
}

/*
 * Exported globals
 */

void bb_init_procs(bb_object *env) {
    bb_bind_proc("%bb-object-type", bb_object_type_proc, env);
    bb_bind_proc("%bb-puts", bb_puts_proc, env);

    bb_bind_proc("+", bb_add_proc, env);
    bb_bind_proc("-", bb_subtract_proc, env);
    bb_bind_proc("*", bb_multiply_proc, env);
    bb_bind_proc("/", bb_divide_proc, env);
    bb_bind_proc("newline", bb_newline_proc, env);
    bb_bind_proc("load", bb_load_proc, env);

    bb_bind_proc("cons", bb_cons_proc, env);
    bb_bind_proc("car", bb_car_proc, env);
    bb_bind_proc("cdr", bb_cdr_proc, env);
    bb_bind_proc("list", bb_list_proc, env);

    bb_bind_proc("=", bb_e_proc, env);
    bb_bind_proc(">", bb_gt_proc, env);
    bb_bind_proc(">=", bb_gte_proc, env);
    bb_bind_proc("<", bb_lt_proc, env);
    bb_bind_proc("<=", bb_lte_proc, env);

    bb_bind_proc("or", bb_or_proc, env);
    bb_bind_proc("and", bb_and_proc, env);

    bb_bind_proc("number->string", bb_number_to_string_proc, env);

    bb_bind_proc("display", bb_display_proc, env);
    bb_bind_proc("void", bb_void_proc, env);
}
