.POSIX:
.PHONY: all clean

SANITIZE = -fsanitize=address,leak,undefined
WARNINGS = -Wall -Wextra -Wpedantic \
	-Wformat=2 -Wno-unused-parameter -Wshadow \
	-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
	-Wredundant-decls -Wnested-externs -Wmissing-include-dirs

CFLAGS = -O0 -g $(SANITIZE) $(WARNINGS)
CFLAGS$(DEBUG) = -O3 -DNDEBUG

all: snake

snake: main.c snake.o window.o map.o term.o
	$(CC) $(CFLAGS) -o $@ $^
snake.o: snake.c snake.h
window.o: window.c snake.h term.h window.h
map.o: map.c map.h snake.h term.h window.h
term.o: term.c term.h


clean:
	rm -f snake *.o
