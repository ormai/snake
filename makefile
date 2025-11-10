.POSIX:
.PHONY: all clean

CC=clang
SANITIZERS = -fsanitize=address,leak,undefined
WARNINGS = -Wall -Wextra -Wpedantic \
	-Wformat=2 -Wno-unused-parameter -Wshadow \
	-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
	-Wredundant-decls -Wnested-externs -Wmissing-include-dirs

# `make DEBUG=1` to enable a sort of dev profile
CFLAGS = -std=c23 -O0 -g $(SANITIZERS) $(WARNINGS)
CFLAGS$(DEBUG) = -std=c23 -O3 -DNDEBUG

all: snake

snake: main.o snake.o window.o map.o term.o
	$(CC) $(CFLAGS) -o $@ $^
main.o: main.c snake.h window.h
snake.o: snake.c snake.h
window.o: window.c snake.h term.h window.h
map.o: map.c map.h snake.h term.h window.h
term.o: term.c term.h

clean:
	rm -f snake *.o
