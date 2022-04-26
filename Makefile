all: fproc gproc main

fproc: src/f.c
	gcc -Wall -o bin/f src/f.c

gproc: src/g.c
	gcc -Wall -o bin/g src/g.c

main: src/main.c
	gcc -Wall -o bin/main src/main.c