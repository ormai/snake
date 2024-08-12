.POSIX:
.PHONY: all clean test

snake: main.c snake.o window.o map.o term.o
	$(CC) $(CFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $<

SANITIZE = -fsanitize=address,leak,undefined
WARNINGS = -Wall -Wextra -Wpedantic \
	-Wformat=2 -Wno-unused-parameter -Wshadow \
	-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
	-Wredundant-decls -Wnested-externs -Wmissing-include-dirs

test:
	$(CC) -Og -ggdb $(SANITIZE) $(WARNINGS) -o snake *.c
	./snake

clean:
	rm -f snake *.o
