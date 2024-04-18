.POSIX:

LDLIBS=-lncursesw
CC=gcc
SANITIZE=-fsanitize=address,alignment,bool,bounds,builtin,enum,vptr,return,null,undefined,unreachable
WARNINGS=-Wall -Wextra -Wpedantic
CFLAGS=-std=c2x $(SANITIZE) $(WARNINGS)

OBJ=screen.o snake.o

snake: src/main.c screen.o snake.o
	$(CC) $(CFLAGS) -o $@ $< $(OBJ) $(LDLIBS)

screen.o: src/screen.c src/screen.h
	$(CC) -c $<
snake.o: src/snake.c src/snake.h
	$(CC) -c $<

run: snake
	./snake

clean:
	rm -f snake *.o
