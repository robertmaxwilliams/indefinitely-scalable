
all:
	makeheaders graphics.c
	python3 prepro.py behave.c > behave.out.c
	makeheaders behave.out.c
	gcc -O3 -std=c99 -Wextra -pedantic-errors graphics.c behave.out.c main.c \
		-lSDL2 -fdiagnostics-color=always -o main
colors:
	python3 color-tabler.py > data_colors.h
