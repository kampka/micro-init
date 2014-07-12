all: clean build

clean:
	rm -f init

build: init

init:
	clang -o init src/init.c

.PHONY: clean build
