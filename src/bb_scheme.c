/* -*- c-basic-offset:4  -*- */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bb_scheme.h"

// todo:
// stop using unsafe string functions
// use hash table for var lookups


/* Eval layer */

static bool bb_is_self_evaluating(bb_object *);
static bool bb_list_begins_with(bb_object *pair, bb_object *search);

bb_object *bb_current_frame(bb_object *env) { return bb_car(env); }
bb_object *bb_parent_env(bb_object *env) { return bb_cdr(env); }
bb_object *bb_make_frame(bb_object *vars, bb_object *vals) {
    return bb_cons(vars, vals);
}
bb_object *bb_frame_vars(bb_object *frame) { return bb_car(frame); }
bb_object *bb_frame_vals(bb_object *frame) { return bb_cdr(frame); }

void bb_frame_bind_var(bb_object *frame, bb_object *var,
                        bb_object *val) {
    bb_set_car(frame, bb_cons(var, bb_car(frame)));
    bb_set_cdr(frame, bb_cons(val, bb_cdr(frame)));
}

static void bb_dump_vars(FILE *file, bb_object *env) {
    //TODO `file` is unused
    int i = 0;
    while(!bb_isemptylist(env)) {
        bb_object *frame = bb_current_frame(env);
        bb_object *var = bb_frame_vars(frame);
        bb_object *val = bb_frame_vals(frame);

        while(!bb_isemptylist(var)) {
            bb_write(bb_car(var));
            printf(" = ");
            bb_write(bb_car(val));
            printf("\n");
            var = bb_cdr(var);
            val = bb_cdr(val);
        }

        env = bb_parent_env(env);
        i++;
    }
}

bb_object *bb_extend_environment(bb_object *vars, bb_object *vals,
                                 bb_object *env) {
    return bb_cons(bb_make_frame(vars, vals), env);
}

bb_object *bb_var_get_value(bb_object *env, bb_object *needle) {
    while(!bb_isemptylist(env)) {
        bb_object *var, *val;
        var = bb_car(bb_car(env));
        val = bb_cdr(bb_car(env));
        while(!bb_isemptylist(var)) {
            if(needle ==  bb_car(var)) {
                return bb_car(val);
            }
            var = bb_cdr(var);
            val = bb_cdr(val);
        }

        env = bb_parent_env(env);
    }
    //TODO write to stderr
    printf("Variable undefined: ");
    bb_write(needle);
    printf("\n");
    exit(-1);
}


static bb_object *bb_define_var(bb_object *env, bb_object *var, bb_object *val) {
    bb_object *frame = bb_current_frame(env);
    bb_object *vars = bb_frame_vars(frame);
    bb_object *vals = bb_frame_vals(frame);
    
    while(!bb_isemptylist(vars)) {
        if(var == bb_car(vars)) {
            bb_set_car(vals, bb_eval(val, env));
            return bb_ok_symbol;
        }
        vars = bb_cdr(vars);
        vals = bb_cdr(vals);
    }
    bb_frame_bind_var(bb_current_frame(env), var, bb_eval(val, env));
    return bb_ok_symbol;
}

static bb_object *bb_set_var(bb_object *env, bb_object *var, bb_object *val) {
    while(!bb_isemptylist(env)) {
        bb_object *frame = bb_current_frame(env);
        bb_object *vars = bb_frame_vars(frame);
        bb_object *vals = bb_frame_vals(frame);
        while(!bb_isemptylist(vars)) {
            if(var == bb_car(vars)) {
                bb_set_car(vals, val);
                return bb_ok_symbol;
            }
            var = bb_cdr(vars);
            val = bb_cdr(vals);
        }

        env = bb_parent_env(env);
        bb_frame_bind_var(bb_current_frame(env), var, bb_eval(val, env));
        
    }
    return bb_ok_symbol;
}

bb_object *bb_if(bb_object *env, bb_object *check, bb_object *yes,
                 bb_object *no) {
    return bb_istrue(bb_eval(check, env)) ? yes : no;
}

// Same as non-recursive solution here:
// http://www.mytechinterviews.com/reverse-a-linked-list
bb_object *bb_reverse_list(bb_object *list) {
    bb_object *temp, *previous = bb_empty_list;
    while(!bb_isemptylist(list)) {
        temp = bb_cdr(list);
        bb_set_cdr(list, previous);
        previous = list;
        list = temp;
    }
    return previous;
}

bb_object *bb_evaluate_list(bb_object *env, bb_object *args, bb_object *acc) {
    if(bb_isemptylist(args)) {
        return bb_reverse_list(acc);
    } else {
        return bb_evaluate_list(env, bb_cdr(args),
                                bb_cons(bb_eval(bb_car(args), env), acc));
    }
}

//TODO check number of arguments given to builtins
bb_object *bb_eval(bb_object *exp, bb_object *env) {

    char comeback = 1;

    while(comeback) {
        comeback = 0;

        if(bb_is_self_evaluating(exp)) {
            return exp;
        }

        if(bb_list_begins_with(exp, bb_quote_symbol)) {
            return bb_cadr(exp);
        }

        // (define... )
        if(bb_list_begins_with(exp, bb_define_symbol)) {
 
            bb_object *var = bb_cadr(exp);

            // (define a b)
            if(bb_issymbol(var)) {
                bb_object *val = bb_caddr(exp);
                return bb_define_var(env, var, val);
            }

            // (define (a ...) ...) TODO use scheme macro
            if(bb_ispair(var)) {
                bb_object *name = bb_car(bb_cadr(exp)),
                    *formals = bb_cdr(bb_cadr(exp)),
                    *body = bb_cddr(exp),
                    *lambda = bb_cons(bb_lambda_symbol,
                                      bb_cons(formals, body));

                exp = bb_cons(bb_define_symbol,
                              bb_cons(name, bb_cons(lambda, bb_empty_list)));
                comeback = 1;
                continue;
            }
            
            fprintf(stderr, "Syntax error.\n");
            exit(-1);
        }

        // (set! a b)
        if(bb_list_begins_with(exp, bb_set_symbol)) {
            bb_object *var = bb_cadr(exp);
            bb_object *val = bb_caddr(exp);
            return bb_set_var(env, var, val);
        }

        // (if c a b)
        if(bb_list_begins_with(exp, bb_if_symbol)) {
            exp = bb_if(env, bb_cadr(exp), bb_caddr(exp), bb_cadddr(exp));
            comeback = 1;
            continue;
        }

        // (cond ...)
        if(bb_list_begins_with(exp, bb_cond_symbol)) {
            bb_object *tail = bb_cons(bb_void_symbol, bb_empty_list);
            bb_object *ifs = tail; //bb_empty_list;
            bb_object *rules = bb_reverse_list(bb_cdr(exp));

            while(!bb_isemptylist(rules)) {
                bb_object *rule = bb_car(rules),
                    *condition = bb_car(rule),
                    *consequence = bb_cadr(rule);

                if(bb_isemptylist(consequence)) {
                    consequence = bb_cons(bb_void, bb_empty_list);
                }
                
                ifs = bb_cons(bb_if_symbol,
                              bb_cons(condition,
                                      bb_cons(consequence,
                                              bb_cons(ifs, bb_empty_list))));

                rules = bb_cdr(rules);
            }

            exp = ifs;

            comeback = 1;
            continue;
        }

        // (begin ...)
        if(bb_list_begins_with(exp, bb_begin_symbol)) {

            bb_object *result = bb_empty_list, *exps;

            for(exps = bb_cdr(exp); ! bb_isemptylist(exps); exps = bb_cdr(exps)) {
                result = bb_eval(bb_car(exps), env);
            }

            return result;
        }

        if(bb_list_begins_with(exp, bb_lambda_symbol)) {
            bb_object *fn = bb_cons(bb_begin_symbol,
                                    bb_cdr(bb_cdr(exp)));
            return bb_make_compound_proc(bb_empty_list, bb_cadr(exp),
                                         fn,
                                         env);
        }

        // (let ...)
        if(bb_list_begins_with(exp, bb_let_symbol)) {
            //if(! bb_issymbol(bb_cadr(exp)))
            bb_object *bindings = bb_cadr(exp);
            bb_object *body = bb_cddr(exp);

            bb_object *formals = bb_empty_list;
            bb_object *values = bb_empty_list;
            
            while(!bb_isemptylist(bindings)) {
                formals = bb_cons(bb_caar(bindings), formals);
                values = bb_cons(bb_cadr(bb_car(bindings)), values);
                
                bindings = bb_cdr(bindings);
            }
            
            exp = bb_cons(bb_cons(bb_lambda_symbol, bb_cons(formals, body)),
                          values);

            comeback = 1;
            continue;
        }

        if(bb_issymbol(exp)) {
            return bb_var_get_value(env, exp);
        }

        if(bb_ispair(exp)) {
            bb_object *car = bb_car(exp);
            bb_object *fn = bb_eval(car, env); //bb_var_get_value(env, car);
            if(!bb_iscallable(fn)) {
                fprintf(stderr, "object is not callable\n");
                exit(-1);
            }

            bb_object *args = bb_cdr(exp);
            bb_object *evaluated_args = bb_evaluate_list(env, args, bb_empty_list);

            if(bb_isprimitiveproc(fn)) {
                return fn->value.prim_proc.fn(evaluated_args);
            } else if(bb_iscompoundproc(fn)) {
                bb_object *fn_formals = fn->value.compound_proc.formals;
                bb_object *fn_body = fn->value.compound_proc.body;
                bb_object *fn_env = fn->value.compound_proc.env;

                BB_ARGS_EQ(evaluated_args, bb_list_size(fn_formals));

                exp = fn_body;
                env = bb_extend_environment(fn_formals, evaluated_args, fn_env);
                comeback = 1;
                continue;

            }
            assert(0);
        }

    }

    fprintf(stderr, "Unable to evaluate expression: \n");
    bb_write(exp);
    exit(-1);
}

static bool bb_is_self_evaluating(bb_object *obj) {
    return bb_isfixnum(obj) || bb_isboolean(obj) || bb_ischaracter(obj) ||
        bb_isstring(obj) || bb_isprimitiveproc(obj);
}

static bool bb_list_begins_with(bb_object *list, bb_object *search) {
    if(bb_ispair(list)) {
        bb_object *pair_car = bb_car(list);
        return bb_issymbol(pair_car) && (pair_car == search);
    }
    return false;
}

bb_object *bb_setup_environment() {
    bb_object *env = bb_extend_environment(bb_empty_list, bb_empty_list,
                                           bb_empty_list);
    return env;
}

//TODO: handle errors
bb_object *bb_load_file(char *filename) {

    FILE *fp = fopen(filename, "r");
    bb_object *forms = bb_empty_list;
    bb_object *form;
    while(!bb_isemptylist(form = bb_read(fp))) {
        forms = bb_cons(form, forms);
    }
    fclose(fp);

    forms = bb_reverse_list(forms);
    forms = bb_cons(bb_begin_symbol, forms);

    //return forms;

    return bb_eval(forms, bb_global_environment);

}

static void on_exit() {
    //TODO after gc is implemented, display max memory used here
    printf("Bye! %d objects in memory (%ld KB).\n\nVars:\n\n", obj_count,
           obj_count * sizeof(bb_object) / 1024);
    //    FILE *dump = fopen("dump.txt", "w");
    //    bb_dump_vars(dump, bb_global_environment);
    //    fclose(dump);
}

/* REPL */
int main() {
    atexit(on_exit);
    bb_init();

    bb_global_environment = bb_setup_environment();
    bb_init_procs(bb_global_environment);

    printf("Welcome to BoboScheme\nUse ctrl-c to exit.\n");

    bb_write(bb_load_file("src/stdlib.scm"));
    printf("\n");

    while(1) {
        printf("> ");
        bb_object *r = bb_read(stdin);
        if(!bb_isemptylist(r)) {
            bb_write(bb_eval( r, bb_global_environment ));
        } else {
            // Ctrl+D
            exit(0);
        }
        printf("\n");
    }

    return 0;
}
