
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bscm.h"


int obj_count = 0;

object_t *alloc_object_t() {
    object_t *obj = (object_t *) malloc(sizeof(object_t));
    obj_count++;
    return obj;
}

bool isfixnum(object_t *o)    { return o->type == FIXNUM; }
bool isboolean(object_t *o)   { return o->type == BOOLEAN; }
bool isfalse(object_t *o)     { return o == false_obj; }
bool istrue(object_t *obj)    { return !isfalse(obj); }
bool ischaracter(object_t *o) { return o->type == CHARACTER; }
bool isstring(object_t *o)    { return o->type == STRING; }
bool isemptylist(object_t *o) { return o->type == EMPTY_LIST; }
bool ispair(object_t *o)      { return o->type == PAIR; }
bool issymbol(object_t *o)    { return o->type == SYMBOL; }
bool isprimitiveproc(object_t *o) { return o->type == PRIMITIVE_PROCEDURE; }
bool iscompoundproc(object_t *o) { return o->type == COMPOUND_PROCEDURE; }

bool iscallable(object_t *o) {
    return isprimitiveproc(o) || iscompoundproc(o);
}

object_t *make_fixnum(long value) {
    object_t *fixnum = alloc_object_t();
    fixnum->type = FIXNUM;
    fixnum->value.fixnum = value;
    return fixnum;
}

object_t *make_bool(bool value) {
    return value ? true_obj : false_obj;
}

object_t *make_character(char value) {
    object_t *character = alloc_object_t();
    character->type = CHARACTER;
    character->value.character = value;
    return character;
}

object_t *make_string(char *value) {
    object_t *string = alloc_object_t();
    string->type = STRING;

    char *str;
    size_t str_size = sizeof(char) * strlen(value) + 1;
    str = (char *) malloc(str_size);
    strncpy(str, value, str_size);
    string->value.string = str;
    return string;
}

object_t *cons(object_t *car, object_t *cdr) {
    object_t *pair = alloc_object_t();
    pair->type = PAIR;
    pair->value.pair.car = car;
    pair->value.pair.cdr = cdr;
    return pair;
}

void set_car(object_t *pair, object_t *value) {
    pair->value.pair.car = value;
}

void set_cdr(object_t *pair, object_t *value) {
    pair->value.pair.cdr = value;
}

object_t *car(object_t *pair) { return pair->value.pair.car; }
object_t *cdr(object_t *pair) { return pair->value.pair.cdr; }

// TODO replace with O(1)
object_t *make_symbol(char *name) {
    object_t *symbol;
    object_t *current; // pointer in symbol table

    for(current = symbol_table;
        ! isemptylist(current);
        current = cdr(current)) {

        if(strcmp(name, car(current)->value.string) == 0) {
            return car(current);
        }

    }

    symbol = alloc_object_t();
    symbol->type = SYMBOL;
    symbol->value.symbol = (char *) malloc(sizeof(char) * strlen(name) + 1);

    symbol_table = cons(symbol, symbol_table);

    strcpy(symbol->value.symbol, name);
    return symbol;
}

// Name is set when using the define syntactic sugar
object_t *make_compound_proc(object_t *name, object_t *formals,
                              object_t *body, object_t *env) {
    object_t *proc = alloc_object_t();
    proc->type = COMPOUND_PROCEDURE;
    proc->value.compound_proc.name = name;
    proc->value.compound_proc.formals = formals;
    proc->value.compound_proc.body = body;
    proc->value.compound_proc.env = env;
    return proc;
}

void init() {
    true_obj = alloc_object_t();
    false_obj = alloc_object_t();

    true_obj->type = false_obj->type = BOOLEAN;
    true_obj->value.boolean = true_obj;
    false_obj->value.boolean = false_obj;

    empty_list = alloc_object_t();
    empty_list->type = EMPTY_LIST;

    void_obj = alloc_object_t();
    void_obj->type = VOID;

    symbol_table = empty_list;
    quote_symbol = make_symbol("quote");

    define_symbol = make_symbol("define");
    set_symbol = make_symbol("set!");
    ok_symbol = make_symbol("ok");
    begin_symbol = make_symbol("begin");
    lambda_symbol = make_symbol("lambda");
    let_symbol = make_symbol("let");

    if_symbol = make_symbol("if");
    cond_symbol = make_symbol("cond");
    void_symbol = make_symbol("void");

    global_environment = setup_environment();
}
