CC := gcc
CFLAGS := -Wpedantic -Wall -lncurses
STDV := -std=c18

all: snake

snake:
	$(CC) $(STDV) $(CFLAGS) src/main.c -o snake

clean:
	$(RM) snake
