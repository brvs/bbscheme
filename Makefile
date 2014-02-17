
CFLAGS = -Wall -g
CC = gcc
OBJS = obj/bb_model.o obj/bb_read.o obj/bb_scheme.o obj/bb_write.o obj/bb_procedures.o

.PHONY: all clean

all: bin/bbscheme

bin/bbscheme: $(OBJS)
	$(CC) $(CFLAGS) -o bin/bbscheme $(OBJS)

obj/bb_model.o: src/bb_scheme.h src/bb_model.c
	$(CC) $(CFLAGS) -c src/bb_model.c -o obj/bb_model.o

obj/bb_read.o: src/bb_scheme.h src/bb_read.c
	$(CC) $(CFLAGS) -c src/bb_read.c -o obj/bb_read.o

obj/bb_write.o: src/bb_scheme.h src/bb_write.c
	$(CC) $(CFLAGS) -c src/bb_write.c -o obj/bb_write.o

obj/bb_procedures.o: src/bb_scheme.h src/bb_procedures.c
	$(CC) $(CFLAGS) -c src/bb_procedures.c -o obj/bb_procedures.o

obj/bb_scheme.o: src/bb_scheme.h src/bb_scheme.c
	$(CC) $(CFLAGS) -c src/bb_scheme.c -o obj/bb_scheme.o

clean:
	rm bin/* obj/*
