.PHONY: all setup clean test
LDLIBS := -lncurses
OBJECTS := build/snake.o build/screen.o

ifeq ($(CONFIG), debug)
	CFLAGS += -g3 -ggdb -Wpedantic -Wall -Wextra
else
	CFLAGS += -O3
endif

all: snake
snake: setup $(OBJECTS)
	$(CC) $(LDLIBS) $(CFLAGS) src/main.c -o $@ $(OBJECTS)

build/snake.o: src/snake.c src/snake.h
	$(CC) $(CFLAGS) -c $< -o $@
build/screen.o: src/screen.c src/screen.h src/snake.h
	$(CC) $(CFLAGS) -c $< -o $@

setup:
	mkdir -p build

test: snake
	./snake

clean:
	$(RM) -r snake build
