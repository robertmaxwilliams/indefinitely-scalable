
all:
	makeheaders graphics.c
	python3 prepro.py main.c > main.out.c
	gcc -O3 -std=c99 -Wextra -pedantic-errors graphics.c\
		main.out.c -lSDL2 -fdiagnostics-color=always -o main
