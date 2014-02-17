
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bb_scheme.h"


int obj_count = 0;

bb_object *bb_alloc_object() {
    bb_object *obj = (bb_object *) malloc(sizeof(bb_object));
    obj_count++;
    return obj;
}

bool bb_isfixnum(bb_object *o)    { return o->type == BB_FIXNUM; }
bool bb_isboolean(bb_object *o)   { return o->type == BB_BOOLEAN; }
bool bb_isfalse(bb_object *o)     { return o == bb_false; }
bool bb_istrue(bb_object *obj)    { return !bb_isfalse(obj); }
bool bb_ischaracter(bb_object *o) { return o->type == BB_CHARACTER; }
bool bb_isstring(bb_object *o)    { return o->type == BB_STRING; }
bool bb_isemptylist(bb_object *o) { return o->type == BB_EMPTY_LIST; }
bool bb_ispair(bb_object *o)      { return o->type == BB_PAIR; }
bool bb_issymbol(bb_object *o)    { return o->type == BB_SYMBOL; }
bool bb_isprimitiveproc(bb_object *o) { return o->type == BB_PRIMITIVE_PROCEDURE; }
bool bb_iscompoundproc(bb_object *o) { return o->type == BB_COMPOUND_PROCEDURE; }

bool bb_iscallable(bb_object *o) {
    return bb_isprimitiveproc(o) || bb_iscompoundproc(o);
}

bb_object *bb_make_fixnum(long value) {
    bb_object *fixnum = bb_alloc_object();
    fixnum->type = BB_FIXNUM;
    fixnum->value.fixnum = value;
    return fixnum;
}

bb_object *bb_make_bool(bool value) {
    return value ? bb_true : bb_false;
}

bb_object *bb_make_character(char value) {
    bb_object *character = bb_alloc_object();
    character->type = BB_CHARACTER;
    character->value.character = value;
    return character;
}

bb_object *bb_make_string(char *value) {
    bb_object *string = bb_alloc_object();
    string->type = BB_STRING;

    char *str;
    size_t str_size = sizeof(char) * strlen(value) + 1;
    str = (char *) malloc(str_size);
    strncpy(str, value, str_size);
    string->value.string = str;
    return string;
}

bb_object *bb_cons(bb_object *car, bb_object *cdr) {
    bb_object *pair = bb_alloc_object();
    pair->type = BB_PAIR;
    pair->value.pair.car = car;
    pair->value.pair.cdr = cdr;
    return pair;
}

void bb_set_car(bb_object *pair, bb_object *value) {
    pair->value.pair.car = value;
}

void bb_set_cdr(bb_object *pair, bb_object *value) {
    pair->value.pair.cdr = value;
}

bb_object *bb_car(bb_object *pair) { return pair->value.pair.car; }
bb_object *bb_cdr(bb_object *pair) { return pair->value.pair.cdr; }

// TODO replace with O(1)
bb_object *bb_make_symbol(char *name) {
    bb_object *symbol;
    bb_object *current; // pointer in symbol table

    for(current = bb_symbol_table;
        ! bb_isemptylist(current);
        current = bb_cdr(current)) {

        if(strcmp(name, bb_car(current)->value.string) == 0) {
            return bb_car(current);
        }

    }

    symbol = bb_alloc_object();
    symbol->type = BB_SYMBOL;
    symbol->value.symbol = (char *) malloc(sizeof(char) * strlen(name) + 1);

    bb_symbol_table = bb_cons(symbol, bb_symbol_table);

    strcpy(symbol->value.symbol, name);
    return symbol;
}

// Name is set when using the define syntactic sugar
bb_object *bb_make_compound_proc(bb_object *name, bb_object *formals,
                              bb_object *body, bb_object *env) {
    bb_object *proc = bb_alloc_object();
    proc->type = BB_COMPOUND_PROCEDURE;
    proc->value.compound_proc.name = name;
    proc->value.compound_proc.formals = formals;
    proc->value.compound_proc.body = body;
    proc->value.compound_proc.env = env;
    return proc;
}

void bb_init() {
    bb_true = bb_alloc_object();
    bb_false = bb_alloc_object();

    bb_true->type = bb_false->type = BB_BOOLEAN;
    bb_true->value.boolean = true;
    bb_false->value.boolean = false;

    bb_empty_list = bb_alloc_object();
    bb_empty_list->type = BB_EMPTY_LIST;

    bb_void = bb_alloc_object();
    bb_void->type = BB_VOID;

    bb_symbol_table = bb_empty_list;
    bb_quote_symbol = bb_make_symbol("quote");

    bb_define_symbol = bb_make_symbol("define");
    bb_set_symbol = bb_make_symbol("set!");
    bb_ok_symbol = bb_make_symbol("ok");
    bb_begin_symbol = bb_make_symbol("begin");
    bb_lambda_symbol = bb_make_symbol("lambda");
    bb_let_symbol = bb_make_symbol("let");

    bb_if_symbol = bb_make_symbol("if");
    bb_cond_symbol = bb_make_symbol("cond");
    bb_void_symbol = bb_make_symbol("void");

    bb_global_environment = bb_setup_environment();
}
