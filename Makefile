
CFLAGS = -Wall -g
CC = gcc
OBJS = obj/model.o obj/read.o obj/bscm.o obj/write.o obj/procedures.o
HEADERS = src/bscm.h

.PHONY: all clean

all: bin/bscm

bin/bscm: $(OBJS)
	$(CC) $(CFLAGS) -o bin/bscm $(OBJS)

obj/model.o: $(HEADERS) src/model.c
	$(CC) $(CFLAGS) -c src/model.c -o obj/model.o

obj/read.o: $(HEADERS) src/read.c
	$(CC) $(CFLAGS) -c src/read.c -o obj/read.o

obj/write.o: $(HEADERS) src/write.c
	$(CC) $(CFLAGS) -c src/write.c -o obj/write.o

obj/procedures.o: $(HEADERS) src/procedures.c
	$(CC) $(CFLAGS) -c src/procedures.c -o obj/procedures.o

obj/bscm.o: $(HEADERS) src/bscm.c
	$(CC) $(CFLAGS) -c src/bscm.c -o obj/bscm.o

clean:
	rm bin/* obj/*
