
C_FLAGS=-O3 -std=c99 -lm -Wextra -fdiagnostics-color=always 
C_SOURCE=build/graphics.c build/behave.out.c build/main.c
SDL2_FILES = /usr/lib/x86_64-linux-gnu/libSDL2.a /usr/lib/x86_64-linux-gnu/libSDL2_image.a /usr/lib/x86_64-linux-gnu/libSDL2main.a /usr/lib/x86_64-linux-gnu/libSDL2_test.a /usr/lib/x86_64-linux-gnu/libSDL2_ttf.a
ELEMENTS = elements/main-cells.c elements/sort.c
		   #elements/tsp.c

CC_SDL=`sdl2-config --cflags --libs`
all: build/makeheaders
	mkdir -p build/
	# run out python preprocessor and colortable maker
	python3 preprocessor.py  src/behave.c  $(ELEMENTS) > build/behave.out.c
	python3 color-tabler.py > build/data_colors.h
	# copy all the files we need to the staging directory
	cp src/graphics.c build/
	cp src/main.c build/
	cp src/constants.h build/
	# make headers 
	./build/makeheaders build/behave.out.c
	./build/makeheaders build/graphics.c
	# compile with SDL2 and optimizations
	gcc $(C_SOURCE) $(C_FLAGS) $(CC_SDL) -o build/main

test:
	curl http://eeweb.poly.edu/~yao/EL5123/image/lena_gray.bmp > build/lena.bmp
	gcc src/test.c $(C_FLAGS) $(CC_SDL) -o build/test

windows-prep:
	mkdir -p build/
	# download sdl2 zip
	curl https://www.libsdl.org/release/SDL2-2.0.12-win32-x64.zip > build/libsdl.zip
	# unzip it
	unzip -d build/ build/libsdl.zip
	# move it to the name the cygwin's sdl lib expects
	mv build/SDL2.dll build/cygSDL2-2-0-0.dll
	# copy in cygwin's main dll in case you didn't put it in your path
	cp /cygdrive/c/cygwin64/bin/cygwin1.dll ./build

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
