
all:
	mkdir -p build/
	# run out python preprocessor and colortable maker
	python3 prepro.py behave.c > build/behave.out.c
	python3 color-tabler.py > build/data_colors.h
	# copy all the files we need to the staging directory
	cp graphics.c build/
	cp main.c build/
	cp constants.h build/
	# make headers
	makeheaders build/behave.out.c
	makeheaders build/graphics.c
	# compile with SDL2 and optimizations
	gcc -O3 -std=c99 -Wextra -pedantic-errors build/*.c \
		-lSDL2 -fdiagnostics-color=always -o main

clean:
	rm -v build/*
