all: build

build:
	gcc -std=gnu99 -g loop.c main.c

run: all
	./a.out
