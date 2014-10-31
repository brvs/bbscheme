
#include <stdio.h>

#include "bscm.h"


void write_pair(object_t *obj) {
    write(obj->value.pair.car);
    if(obj->value.pair.cdr == empty_list) {
    } else if(ispair(obj->value.pair.cdr)) {
        printf(" ");
        write_pair(obj->value.pair.cdr);
    } else {
        printf(" . ");
        write(obj->value.pair.cdr);
    }
}

void write(object_t *obj) {
    char *start;
    switch(obj->type) {
    case FIXNUM:
        printf("%ld", obj->value.fixnum);
        break;
    case BOOLEAN:
        printf("#%c", obj == true_obj ? 't' : 'f');
        break;
    case CHARACTER:
        printf("#\\");
        switch(obj->value.character) {
            case '\t': printf("tab"); break;
            case '\n': printf("newline"); break;
            case ' ': printf("space"); break;
            default: printf("%c", obj->value.character);
        }
        break;
    case STRING:
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
    case EMPTY_LIST:
        printf("()");
        break;
    case PAIR:
        printf("(");
        write_pair(obj);
        printf(")");
        break;
    case SYMBOL:
        printf("%s", obj->value.symbol);
        break;
    case PRIMITIVE_PROCEDURE:
        printf("<primitive proc: ");
        write(obj->value.prim_proc.name);
        printf(">");
        break;
    case COMPOUND_PROCEDURE:
        printf("<lambda: ");
        if(isemptylist(obj->value.compound_proc.name) ) {
            printf("anonymous %p", (void*)obj);
        } else {
            write(obj->value.compound_proc.name);
        }
        printf(">");
        break;
    case VOID:
        break;
    default:
        fprintf(stderr, "Unknown object_t type: %d\n", obj->type);
    }
}

