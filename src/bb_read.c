
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bb_scheme.h"


bool bb_isdelimeter(int c) {
    return isspace(c) || c == EOF || c == '(' || c == ')' || c == ';';
}

bool bb_issymbolchar(int c) {
    return isalpha(c) || c == '-' || c == '+' || c == '/' || c == '*'
        || c == '>' || c == '<' || c == '=' || c == '!' || c == '?'
        || c == '_' || c == '%';
}

int bb_peek(FILE *fin) {
    int c = getc(fin);
    ungetc(c, fin);
    return c;
}

int bb_skip_comment(FILE *file) {
    int lastc = 0, i = 0;
    while(((lastc = getc(file)) != '\n') && lastc != EOF) {
        i++;
    }
    return i;
}

int bb_skip_whitespace(FILE *file) {
    int lastc = 0;
    int i = 0;
    while(isspace(lastc = getc(file)) || lastc == ';') {
        if(lastc == ';') {
            i += bb_skip_comment(file);
        } else {
            i++;
        }
    }

    ungetc(lastc, file);
    return i;
}

void bb_peek_isdelimeter(FILE *fin) {
    if(!bb_isdelimeter(bb_peek(fin))) {
        fprintf(stderr, "Expecting delimeter after symbol end");
        exit(-1);
    }
}

bb_object *bb_read_pair(FILE *fin) {
    bb_skip_whitespace(fin);
    int c = getc(fin);
    if(c == ')') {
        return bb_empty_list;
    }

    ungetc(c, fin);
    bb_object *car = bb_read(fin), *cdr;
    bb_skip_whitespace(fin);
    c = getc(fin);
    if(c == '.') {
        bb_skip_whitespace(fin);
        cdr = bb_read(fin);
        bb_skip_whitespace(fin);
        if((c = getc(fin)) == ')') {
            return bb_cons(car, cdr);
        }
        fprintf(stderr, "Malformed pair\n");
        exit(-1);
    } else {
        ungetc(c, fin);
        cdr = bb_read_pair(fin);
        //fprintf(stderr, "Invalid pair\n");
        //exit(-1);
    }

    return bb_cons(car, cdr);
}

// fin is positioned before a fixnum
bb_object *bb_read_fixnum(FILE *fin) {
    int c = getc(fin);
    int num = 0;
    int sign = 1;

    if(c == '-') {
        sign = -1;
    } else {
        num = c - '0';
    }

    while(isdigit(c = getc(fin))) {
        num = num * 10 + c - '0';
    }

    ungetc(c, fin);
    bb_peek_isdelimeter(fin);
    return bb_make_fixnum(sign * num);
}

// fin is positioned right before a character, after # and backslash
bb_object *bb_read_character(FILE *fin) {
    char buffer[16] = {0};
    int i, c = 0;
    buffer[0] = getc(fin);
    for(i = 1; !bb_isdelimeter(c = getc(fin)) && i < 16; i++) {
        buffer[i] = c;
    }
    ungetc(c, fin);
    if(strlen(buffer) == 1) {
        return bb_make_character(buffer[0]);
    }
    if(strncmp(buffer, "newline", 8) == 0) {
        return bb_make_character('\n');
    }
    if(strncmp(buffer, "space", 6) == 0) {
        return bb_make_character(' ');
    }
    if(strncmp(buffer, "tab", 4) == 0) {
        return bb_make_character('\t');
    }
    fprintf(stderr, "Invalid character code: %s\n", buffer);
    exit(-1);
}

bb_object *bb_read(FILE *fin) {
    char buffer[4096] = {0};
    int c = -1;

    bb_skip_whitespace(fin);

    int first = c = getc(fin);
    
    if(first == EOF) {
        return bb_empty_list;
    } else if(isdigit(first) || (first == '-' && isdigit(bb_peek(fin)))) { // Fixnum
        ungetc(c, fin);
        return bb_read_fixnum(fin);
    } else if(first == '#') { // Character or boolean
        c = getc(fin);
        if(c == 't' && bb_isdelimeter(bb_peek(fin))) {
            return bb_true;
        } else if(c == 'f' && bb_isdelimeter(bb_peek(fin))) {
            return bb_false;
        } else if(c == '\\') { // character
            return bb_read_character(fin);
        } else {
            fprintf(stderr, "Only t or f or \\ can follow #.\n");
            exit(-1);
        }
    // string
    } else if(first == '"') {
        // TODO allow strings to be huge
        int i;

        for(i = 0; (c = getc(fin)) != '"' && i < sizeof(buffer); i++) {
            if(c == '\\') {
                c = getc(fin); // get escape code
                if(c == '"') {
                    buffer[i] = '"';
                } else if(c == 'n') {
                    buffer[i] = '\n';
                } else if(c == '\\') {
                    buffer[i] = '\\';
                } else if(c == 't') {
                    buffer[i] = '\t';
                } else {
                    fprintf(stderr, "Unknown string escape code: %c\n", c);
                    exit(-1);
                }
            } else {
                buffer[i] = c;
            }
        }
        bb_peek_isdelimeter(fin);
        return bb_make_string(buffer);
    // pair
    } else if(first == '(') {
        return bb_read_pair(fin);
    } else if(bb_issymbolchar(first)) {
        buffer[0] = first;
        int i = 1;
        while(bb_issymbolchar(c = getc(fin)) || isdigit(c)) {
            buffer[i++] = c;
        }
        ungetc(c, fin);
        bb_peek_isdelimeter(fin);
        return bb_make_symbol(buffer);
    } else if(first == '\'') {
        return bb_cons(bb_quote_symbol, bb_cons(bb_read(fin), bb_empty_list));
    } else {
        fprintf(stderr, "FATAL: Unknown symbol: %c\n", (char)first);
        exit(-1);
    }

    /* Should have returned a value before now */
    fprintf(stderr, "Read error: bad state\n");
    assert(0);
    exit(-1);
    return NULL;
}
