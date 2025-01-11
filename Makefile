CC=gcc
CFLAGS=-I./include -std=gnu99 -Wall -Wextra -g #-DDEBUG
LDFLAGS=-lncurses -lsqlite3

INC=./include
SRC=./src
OBJ=./obj
DOC=./doc
BIN=./bin

vpath %.h ./include
vpath %.c ./src

EXECS = main

all: mkpaths $(EXECS)

mkpaths:
	@ mkdir -p $(OBJ)
	@ mkdir -p $(BIN)
doc:
	doxygen

main: main.c
	@ mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(SRC)/main.c -o $(BIN)/$@ $(LDFLAGS)

.PHONY: clean
clean:
	/bin/rm -rf $(BIN)/* $(OBJ)/* core* *~ $(SRC)/*~ $(INC)/*~ $(DOC)/*

