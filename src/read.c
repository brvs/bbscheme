
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bscm.h"


bool isdelimeter(int c) {
    return isspace(c) || c == EOF || c == '(' || c == ')' || c == ';';
}

bool issymbolchar(int c) {
    return isalpha(c) || c == '-' || c == '+' || c == '/' || c == '*'
        || c == '>' || c == '<' || c == '=' || c == '!' || c == '?'
        || c == '_' || c == '%';
}

int peek(FILE *fin) {
    int c = getc(fin);
    ungetc(c, fin);
    return c;
}

int skip_comment(FILE *file) {
    int lastc = 0, i = 0;
    while(((lastc = getc(file)) != '\n') && lastc != EOF) {
        i++;
    }
    return i;
}

int skip_whitespace(FILE *file) {
    int lastc = 0;
    int i = 0;
    while(isspace(lastc = getc(file)) || lastc == ';') {
        if(lastc == ';') {
            i += skip_comment(file);
        } else {
            i++;
        }
    }

    ungetc(lastc, file);
    return i;
}

void peek_isdelimeter(FILE *fin) {
    if(!isdelimeter(peek(fin))) {
        fprintf(stderr, "Expecting delimeter after symbol end");
        exit(-1);
    }
}

object_t *read_pair(FILE *fin) {
    skip_whitespace(fin);
    int c = getc(fin);
    if(c == ')') {
        return empty_list;
    }

    ungetc(c, fin);
    object_t *car = read(fin), *cdr;
    skip_whitespace(fin);
    c = getc(fin);
    if(c == '.') {
        skip_whitespace(fin);
        cdr = read(fin);
        skip_whitespace(fin);
        if((c = getc(fin)) == ')') {
            return cons(car, cdr);
        }
        fprintf(stderr, "Malformed pair\n");
        exit(-1);
    } else {
        ungetc(c, fin);
        cdr = read_pair(fin);
        //fprintf(stderr, "Invalid pair\n");
        //exit(-1);
    }

    return cons(car, cdr);
}

// fin is positioned before a fixnum
object_t *read_fixnum(FILE *fin) {
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
    peek_isdelimeter(fin);
    return make_fixnum(sign * num);
}

// fin is positioned right before a character, after # and backslash
object_t *read_character(FILE *fin) {
    char buffer[16] = {0};
    int i, c = 0;
    buffer[0] = getc(fin);
    for(i = 1; !isdelimeter(c = getc(fin)) && i < 16; i++) {
        buffer[i] = c;
    }
    ungetc(c, fin);
    if(strlen(buffer) == 1) {
        return make_character(buffer[0]);
    }
    if(strncmp(buffer, "newline", 8) == 0) {
        return make_character('\n');
    }
    if(strncmp(buffer, "space", 6) == 0) {
        return make_character(' ');
    }
    if(strncmp(buffer, "tab", 4) == 0) {
        return make_character('\t');
    }
    fprintf(stderr, "Invalid character code: %s\n", buffer);
    exit(-1);
}

object_t *read(FILE *fin) {
    char buffer[4096] = {0};
    int c = -1;

    skip_whitespace(fin);

    int first = c = getc(fin);

    if(first == EOF) {
        return empty_list;
    } else if(isdigit(first) || (first == '-' && isdigit(peek(fin)))) { // Fixnum
        ungetc(c, fin);
        return read_fixnum(fin);
    } else if(first == '#') { // Character or boolean
        c = getc(fin);
        if(c == 't' && isdelimeter(peek(fin))) {
            return true_obj;
        } else if(c == 'f' && isdelimeter(peek(fin))) {
            return false_obj;
        } else if(c == '\\') { // character
            return read_character(fin);
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
        peek_isdelimeter(fin);
        return make_string(buffer);
    // pair
    } else if(first == '(') {
        return read_pair(fin);
    } else if(issymbolchar(first)) {
        buffer[0] = first;
        int i = 1;
        while(issymbolchar(c = getc(fin)) || isdigit(c)) {
            buffer[i++] = c;
        }
        ungetc(c, fin);
        peek_isdelimeter(fin);
        return make_symbol(buffer);
    } else if(first == '\'') {
        return cons(quote_symbol, cons(read(fin), empty_list));
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
