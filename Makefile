
all: build/makeheaders
	mkdir -p build/
	# run out python preprocessor and colortable maker
	python3 prepro.py behave.c > build/behave.out.c
	python3 color-tabler.py > build/data_colors.h
	# copy all the files we need to the staging directory
	cp graphics.c build/
	cp main.c build/
	cp constants.h build/
	# make headers 
	./build/makeheaders build/behave.out.c
	./build/makeheaders build/graphics.c
	# compile with SDL2 and optimizations
	gcc -O3 -std=c99 -Wextra -pedantic-errors build/graphics.c build/behave.out.c build/main.c \
		-lSDL2 -fdiagnostics-color=always -o main

build/makeheaders:
	mkdir -p build/
	# download makeheaders.c
	curl https://fossil-scm.org/fossil/raw/a8afb0c00219eec02369ad1c1446e8a5f9d82c60e6f83d95cdc8511fc7001371?at=makeheaders.c \
		> build/makeheaders.c
	touch build/makeheaders.c
	# compile makeheaders.c
	gcc build/makeheaders.c -std=c99 -o build/makeheaders


clean:
	rm -v build/*
