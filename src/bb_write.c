
#include <stdio.h>

#include "bb_scheme.h"


void bb_write_pair(bb_object *obj) {
    bb_write(obj->value.pair.car);
    if(obj->value.pair.cdr == bb_empty_list) {
    } else if(bb_ispair(obj->value.pair.cdr)) {
        printf(" ");
        bb_write_pair(obj->value.pair.cdr);
    } else {
        printf(" . ");
        bb_write(obj->value.pair.cdr);
    }
}

void bb_write(bb_object *obj) {
    char *start;
    switch(obj->type) {
    case BB_FIXNUM:
        printf("%ld", obj->value.fixnum);
        break;
    case BB_BOOLEAN:
        printf("#%c", obj == bb_true ? 't' : 'f');
        break;
    case BB_CHARACTER:
        printf("#\\");
        switch(obj->value.character) {
            case '\t': printf("tab"); break;
            case '\n': printf("newline"); break;
            case ' ': printf("space"); break;
            default: printf("%c", obj->value.character);
        }
        break;
    case BB_STRING:
        putchar('"');
        for(start = obj->value.string; *start != '\0'; start++) {
            if(*start == '\n') {
                printf("\\n");
            } else if(*start == '"') {
                printf("\\\"");
            } else {
                putchar(*start);
            }
        }
        putchar('"');
        break;
    case BB_EMPTY_LIST:
        printf("()");
        break;
    case BB_PAIR:
        printf("(");
        bb_write_pair(obj);
        printf(")");
        break;
    case BB_SYMBOL:
        printf("%s", obj->value.symbol);
        break;
    case BB_PRIMITIVE_PROCEDURE:
        printf("<primitive proc: ");
        bb_write(obj->value.prim_proc.name);
        printf(">");
        break;
    case BB_COMPOUND_PROCEDURE:
        printf("<lambda: ");
        if(bb_isemptylist(obj->value.compound_proc.name) ) {
            printf("anonymous %p", (void*)obj);
        } else {
            bb_write(obj->value.compound_proc.name);
        }
        printf(">");
        break;
    case BB_VOID:
        break;
    default:
        fprintf(stderr, "Unknown object type: %d\n", obj->type);
    }
}

