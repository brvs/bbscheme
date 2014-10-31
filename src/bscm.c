#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bscm.h"

// todo:
// stop using unsafe string functions
// use hash table for var lookups


/* Eval layer */

static bool is_self_evaluating(object_t *);
static bool list_begins_with(object_t *pair, object_t *search);

object_t *current_frame(object_t *env) { return car(env); }
object_t *parent_env(object_t *env) { return cdr(env); }
object_t *make_frame(object_t *vars, object_t *vals) {
    return cons(vars, vals);
}
object_t *frame_vars(object_t *frame) { return car(frame); }
object_t *frame_vals(object_t *frame) { return cdr(frame); }

void frame_bind_var(object_t *frame, object_t *var,
                        object_t *val) {
    set_car(frame, cons(var, car(frame)));
    set_cdr(frame, cons(val, cdr(frame)));
}

static void dump_vars(FILE *file, object_t *env) {
    //TODO `file` is unused
    int i = 0;
    while(!isemptylist(env)) {
        object_t *frame = current_frame(env);
        object_t *var = frame_vars(frame);
        object_t *val = frame_vals(frame);

        while(!isemptylist(var)) {
            write(car(var));
            printf(" = ");
            write(car(val));
            printf("\n");
            var = cdr(var);
            val = cdr(val);
        }

        env = parent_env(env);
        i++;
    }
}

object_t *extend_environment(object_t *vars, object_t *vals,
                                 object_t *env) {
    return cons(make_frame(vars, vals), env);
}

object_t *var_get_value(object_t *env, object_t *needle) {
    while(!isemptylist(env)) {
        object_t *var, *val;
        var = car(car(env));
        val = cdr(car(env));
        while(!isemptylist(var)) {
            if(needle ==  car(var)) {
                return car(val);
            }
            var = cdr(var);
            val = cdr(val);
        }

        env = parent_env(env);
    }
    //TODO write to stderr
    printf("Variable undefined: ");
    write(needle);
    printf("\n");
    exit(-1);
}


static object_t *define_var(object_t *env, object_t *var, object_t *val) {
    object_t *frame = current_frame(env);
    object_t *vars = frame_vars(frame);
    object_t *vals = frame_vals(frame);

    while(!isemptylist(vars)) {
        if(var == car(vars)) {
            set_car(vals, eval(val, env));
            return ok_symbol;
        }
        vars = cdr(vars);
        vals = cdr(vals);
    }
    frame_bind_var(current_frame(env), var, eval(val, env));
    return ok_symbol;
}

static object_t *set_var(object_t *env, object_t *var, object_t *val) {
    while(!isemptylist(env)) {
        object_t *frame = current_frame(env);
        object_t *vars = frame_vars(frame);
        object_t *vals = frame_vals(frame);
        while(!isemptylist(vars)) {
            if(var == car(vars)) {
                set_car(vals, val);
                return ok_symbol;
            }
            var = cdr(vars);
            val = cdr(vals);
        }

        env = parent_env(env);
        frame_bind_var(current_frame(env), var, eval(val, env));

    }
    return ok_symbol;
}

object_t *eval_if(object_t *env, object_t *check, object_t *yes,
                 object_t *no) {
    return istrue(eval(check, env)) ? yes : no;
}

// Same as non-recursive solution here:
// http://www.mytechinterviews.com/reverse-a-linked-list
object_t *reverse_list(object_t *list) {
    object_t *temp, *previous = empty_list;
    while(!isemptylist(list)) {
        temp = cdr(list);
        set_cdr(list, previous);
        previous = list;
        list = temp;
    }
    return previous;
}

object_t *evaluate_list(object_t *env, object_t *args, object_t *acc) {
    if(isemptylist(args)) {
        return reverse_list(acc);
    } else {
        return evaluate_list(env, cdr(args),
                                cons(eval(car(args), env), acc));
    }
}

//TODO check number of arguments given to builtins
object_t *eval(object_t *exp, object_t *env) {

    char comeback = 1;

    while(comeback) {
        comeback = 0;

        if(is_self_evaluating(exp)) {
            return exp;
        }

        if(list_begins_with(exp, quote_symbol)) {
            return cadr(exp);
        }

        // (define... )
        if(list_begins_with(exp, define_symbol)) {

            object_t *var = cadr(exp);

            // (define a b)
            if(issymbol(var)) {
                object_t *val = caddr(exp);
                return define_var(env, var, val);
            }

            // (define (a ...) ...) TODO use scheme macro
            if(ispair(var)) {
                object_t *name = car(cadr(exp)),
                    *formals = cdr(cadr(exp)),
                    *body = cddr(exp),
                    *lambda = cons(lambda_symbol,
                                      cons(formals, body));

                exp = cons(define_symbol,
                              cons(name, cons(lambda, empty_list)));
                comeback = 1;
                continue;
            }

            fprintf(stderr, "Syntax error.\n");
            exit(-1);
        }

        // (set! a b)
        if(list_begins_with(exp, set_symbol)) {
            object_t *var = cadr(exp);
            object_t *val = caddr(exp);
            return set_var(env, var, val);
        }

        // (if c a b)
        if(list_begins_with(exp, if_symbol)) {
            exp = eval_if(env, cadr(exp), caddr(exp), cadddr(exp));
            comeback = 1;
            continue;
        }

        // (cond ...)
        if(list_begins_with(exp, cond_symbol)) {
            object_t *tail = cons(void_symbol, empty_list);
            object_t *ifs = tail; //empty_list;
            object_t *rules = reverse_list(cdr(exp));

            while(!isemptylist(rules)) {
                object_t *rule = car(rules),
                    *condition = car(rule),
                    *consequence = cadr(rule);

                if(isemptylist(consequence)) {
                    consequence = cons(void_obj, empty_list);
                }

                ifs = cons(if_symbol,
                              cons(condition,
                                      cons(consequence,
                                              cons(ifs, empty_list))));

                rules = cdr(rules);
            }

            exp = ifs;

            comeback = 1;
            continue;
        }

        // (begin ...)
        if(list_begins_with(exp, begin_symbol)) {

            object_t *result = empty_list, *exps;

            for(exps = cdr(exp); ! isemptylist(exps); exps = cdr(exps)) {
                result = eval(car(exps), env);
            }

            return result;
        }

        if(list_begins_with(exp, lambda_symbol)) {
            object_t *fn = cons(begin_symbol,
                                    cdr(cdr(exp)));
            return make_compound_proc(empty_list, cadr(exp),
                                         fn,
                                         env);
        }

        // (let ...)
        if(list_begins_with(exp, let_symbol)) {
            //if(! issymbol(cadr(exp)))
            object_t *bindings = cadr(exp);
            object_t *body = cddr(exp);

            object_t *formals = empty_list;
            object_t *values = empty_list;

            while(!isemptylist(bindings)) {
                formals = cons(caar(bindings), formals);
                values = cons(cadr(car(bindings)), values);

                bindings = cdr(bindings);
            }

            exp = cons(cons(lambda_symbol, cons(formals, body)),
                          values);

            comeback = 1;
            continue;
        }

        if(issymbol(exp)) {
            return var_get_value(env, exp);
        }

        if(ispair(exp)) {
            object_t *exp_car = car(exp);
            object_t *fn = eval(exp_car, env); //var_get_value(env, car);
            if(!iscallable(fn)) {
                fprintf(stderr, "object_t is not callable\n");
                exit(-1);
            }

            object_t *args = cdr(exp);
            object_t *evaluated_args = evaluate_list(env, args, empty_list);

            if(isprimitiveproc(fn)) {
                return fn->value.prim_proc.fn(evaluated_args);
            } else if(iscompoundproc(fn)) {
                object_t *fn_formals = fn->value.compound_proc.formals;
                object_t *fn_body = fn->value.compound_proc.body;
                object_t *fn_env = fn->value.compound_proc.env;

                ARGS_EQ(evaluated_args, list_size(fn_formals));

                exp = fn_body;
                env = extend_environment(fn_formals, evaluated_args, fn_env);
                comeback = 1;
                continue;

            }
            assert(0);
        }

    }

    fprintf(stderr, "Unable to evaluate expression: \n");
    write(exp);
    exit(-1);
}

static bool is_self_evaluating(object_t *obj) {
    return isfixnum(obj) || isboolean(obj) || ischaracter(obj) ||
        isstring(obj) || isprimitiveproc(obj);
}

static bool list_begins_with(object_t *list, object_t *search) {
    if(ispair(list)) {
        object_t *pair_car = car(list);
        return issymbol(pair_car) && (pair_car == search);
    }
    return false;
}

object_t *setup_environment() {
    object_t *env = extend_environment(empty_list, empty_list,
                                           empty_list);
    return env;
}

//TODO: handle errors
object_t *load_file(char *filename) {

    FILE *fp = fopen(filename, "r");
    object_t *forms = empty_list;
    object_t *form;
    while(!isemptylist(form = read(fp))) {
        forms = cons(form, forms);
    }
    fclose(fp);

    forms = reverse_list(forms);
    forms = cons(begin_symbol, forms);

    //return forms;

    return eval(forms, global_environment);

}

static void on_exit() {
    //TODO after gc is implemented, display max memory used here
    printf("Bye! %d object_ts in memory (%ld KB).\n\nVars:\n\n", obj_count,
           obj_count * sizeof(object_t) / 1024);
    //    FILE *dump = fopen("dump.txt", "w");
    //    dump_vars(dump, global_environment);
    //    fclose(dump);
}

/* REPL */
int main() {
    atexit(on_exit);
    init();

    global_environment = setup_environment();
    init_procs(global_environment);

    printf("Welcome to BoboScheme\nUse ctrl-c to exit.\n");

    write(load_file("src/stdlib.scm"));
    printf("\n");

    while(1) {
        printf("> ");
        object_t *r = read(stdin);
        if(!isemptylist(r)) {
            write(eval( r, global_environment ));
        } else {
            // Ctrl+D
            exit(0);
        }
        printf("\n");
    }

    return 0;
}
