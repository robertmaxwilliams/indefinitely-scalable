
all:
	makeheaders graphics.c
	gcc -O3 -std=c99 -Wextra -pedantic-errors graphics.c foo.c -lSDL2 -fdiagnostics-color=always
