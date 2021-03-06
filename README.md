
# Installation

```
make
./build/main
```

Requirements:
- python3
- SDL2
- makeheaders (todo download and build this requirement from web)
- standard \*nix tools: make, gcc, curl. Honestly, if your computer doesn't have
those it's hard to call it a computer.

## Windows Instructions (cygwin)

If you're on windows, go to the bottom of the readme and follow those intructions carefully.

# Terminology

Each square on the grid is a "cell" (or sometimes "atom") and each cell has a type, which is one of
the "elements". What element a cell is determines its behavior - how it acts, if it moves, what
other cells will do to it. Cells also have data, which nearby cells can read or edit and which is
the only state a cell preserves between updates. The update function is called on one cell every
step, and with enough steps this gives the impression of a constantly evolving world, but no two
nearby cells are ever actually updated at the same time, like in Conway's Game of Life.

# User interface

The user interface makes use of both the graphical window and the text terminal to provide the best
experience with minimal tooling.

Left click to draw, click on the right sidebar to choose the active element. The name of that
element is printed out.

Middle click to draw a big box of the same element.

Right click to have cell information printed out in the terminal

Each cell is drawn with the outside color being its type and the inside color one of its data
elements.


Keyboard shortcuts:
- (number 0-9): selects on of the first 10 elements as active
- `q` or escape: quit
- `s`: save world to `save.dat`
- `r`: load world from `save.dat`
- `g`: blast the world with gamma rays (random bit flips)
- `h`: print out any "string" stored on the board
- `i`: increase the size of the drawing tool
- `u`: deacrease the size of the drawing tool
- `j`: increment which data element is displayed in the center of the cell
- `k`: decrement which data element is displayed in the center of the cell
- `.`: increase the simulation speed by 10x (be careful, this will cause fps to drop)
- `,`: decrease the simulation speed by 10x

Terminal:
- Press enter with nothing to print out Average Event Rate, Frames Per Second and other info
- Type something and press enter to write that string into the world wherever you last clicked.

# Changing size

There's no way to change size while it is running, but if you open `constants.h` you can 
change `WORLD_SIZE` to get a bigger world and `SCALE` to choose how many pixels each cell takes up.

# Build process

Because I'm lazy, the build process is a bit convoluted. Thankfully
it's all run by a makefile so hopefully you don't have to worry about
that.

`src/behave.c` defines helpers for all the atom types (aka elements) and the update functions.

`elements/` contains code for each of the elements and #CELL tags for their names and colors.

`behave.c` and the `.c` files in `elements/` are preprocessed by `preprocessor.py` to make
`build/behave.out.c`

`color-tabler.py` is used to make `data_colors.h`, which is just a constant
array used for graphics.

`src/constants.h` keeps all the struct types and global #defines and such that
need tweaking. The sizes of everything is mostly kept in here.

`main.c`, `constants.h` and `graphics.c` are copied from `src/` to `build/`

Once all that is in place, `makeheaders` makes a header for `build/graphics.c` and
`build/behave.out.c`

Then `main.c`, `graphics.c`, and `behave.out.c` are all compiled to make
the program, `main`.

TODO draw a flowchart

# Modifying/adding new elements

The elements are all kept in C files in 'elements/', you can edit these files or
make your own but be careful editing `main-cells.c` because `BLANK` and `STRING` have to be first
and second, respectively.

Also, keep `behave.c` open, since you have access to all of its helper functions.

Now define your new element using a #CELL tag on its own line and by defining a function on the very
next line:

```
#ELEMENT SOME_NAME
void doesnt_matter_what_you_name_it(cell_t* cell) {
    // your code goes here
}
```

You can also put a color after the name (I got mine from
[htmlcolorcodes.com](https://htmlcolorcodes.com/)) like this:

```
#ELEMENT SOME_NAME 0xD4AC0D
```

otherwise a color will be choosen randomly for you.

Also, you can define new helper functions in `behave.c` or in the elements' source files, they all
get combined together so it doesn't make a big difference.

If you make a new `.c` file in elements, you'll have to add it to the makefile, to the variable
called `ELEMENTS`. Again, it's important that `main-cells.c` comes first, and the order everything
is in will determine what order they'll show up in the right sidebar. Also, if the sidebar is full
you won't be able to click on your element. Fix this by increasing `NUM_SIDEBAR_COLUMNS` in
`src/constants.c`

## The ascii art thing (PATTERN)

When `prepro.py` sees the string "PATTERN", it interprets everything between PATTERN
and the next semicolon as ascii art describing which cells to assign to the single
letter variable names in the ascii art. To write a pattern, first put the string PATTERN on its
own line, the make subsequent lines start with `|`, followed by any letters. Make sure there is one
and only one `.` in the pattern, then end it with a semicolon. Here is an example:

```
PATTERN
    |  aa
    | x.
    |  yz;
```

This will cause the variable `x` to be assigned to a cell to the left of the current one 
(which MUST be named `cell`), `y` to the one below, and `z` below and to the left. You can
go as far as you want, but remember that real scalable implementations won't allow for arbitrarily
large "sight radius" so keep it small. The letter `a` is repeated, so it is randomly assigned to one
of its two positions. You can repeat a variable as many times as you like.

This syntax is designed to work with a C auto formatter (vim's is the one I'm using) to indent
correctly.

You can also put "ROTATE foo" (where foo is any variable or expression) on the same line as
"PATTERN" to have the pattern be rotated by that amount. This can be used to do random rotations or
those based on some state of the cell. For example, the following:

```
PATTERN ROTATE cell->data[9]
    |  ab
    |  .;
```

is used to scan the 8 cells around the active site, only looking at two each update.


# How it works and program-internal terminology

The grid you see on the screen is simply the `world`. Each square is called a 
`cell`, although in literature the smallest unit is called an `atom`. I use the
term `atom` when talking about the moveable elements that inhabit cells, but
for the program everything in the cell is just `cell`. 

Each cell is a struct which has 1 byte for its `type` (which chooses its color and `update_`
function), 10 bytes of state in `data`, and a pointer to each of its four neighbors.

The main program picks a cell at random from the world, this is the "active site". It then performs
an update by calling the update function corresponding to that cell's type.

The cells at the edge of the world point off to the "hole" cell, of which there is only one and
which only points to itself. It is much like NULL, except it's left/right/up/down member point to 
itself which makes programming much easier. Checking for "hole" is the same as checking for being at
the edge of the world. If an atom modifies hole's state or type, it should be reset to prevent
instantanious information transmission.


# BIG todo's

- Allow non-square worlds
- Allow zooming and window resizing
- Make it possible to hook up multiple running programs over a pipe and make a bigger grid!!
- That last one would put us very close to stage 3.


# Windows Instructions (cygwin)

Download 64 bit cygwin: <https://cygwin.com/setup-x86_64.exe>

Run the installer, click next, next , 
next (install to C:\cygwin64 !! this is important to leave alone), next (download from internet), 
next (choose a mirror, I just use the first one). Now you are on the "choose packages" screen. 
If you ever need more packages, re-run the installer and come here. For now, choose "Full" in the view
dropdown, and search each of the following packages. Some have repeates so make sure the description
matches. To mark a package for installation, look at the "New" column where most packages say "Skipped"
and double click it to get the default version.

```
make : The GNU version of the 'make' utility
git: Distributed version control system
curl: Multi-protocol file transfer tool
libSDL2-devel: Simple DirectMedia Layer core library
gcc-core : GNU Compiler Collection (C, OpenMP)
unzip: Info-ZIP decompression utility
python3: Meta-package for python 3 default version
```

Hit next and it will install cygwin and the packages. Finally, check that you want a desktop or start menu
shortcut or whatver, finish the installer, and launch cygwin. Clone this repo into your cygwin home dir and
then go into the repo by running:

```
git clone https://github.com/robertmaxwilliams/indefinitely-scalable.git
cd indefinitely-scalable
```

Now run the following to put the correct dll's in the build directory (some are downloaded, some are
copied from C:\cygwin64\bin :

```
make windows-prep
```

Finally you can make and run the program as if you were using an actual computer and not some cobbled 
together hodge podge designed for the illiterate and statistics majors:

```
make
./build/main
```

To run again, you'll have to launch cygwin, `cd` into the repo, and `./build/main`, because double-clicking
on the exe doesn't give you a proper shell so trying to print to the shell causes it to break.

Also, if you `make clean` you'll have to `make windows-prep` again.
