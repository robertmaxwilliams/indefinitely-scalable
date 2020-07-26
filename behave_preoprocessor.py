import itertools
import re
import argparse
import random

import ascii_art
# This preprocessor takes multiple behavior files (with .be.c extension)
# and prints out a single c file that combines them and creates color and enums:

'''
//                 0      1          2         3    4    5     6    7      8   9
enum CELL_TYPES {BLANK=0, STRING=1, ISOLATER, DREG, RES, WALL, REM, MAKER, BOX, STRING_COPIER, 
    DOWN_DROPPER, SHORT_LIVED, FALLER, STRINGS_SPLITTER, RISER}; 

unsigned int cell_colors[256] = {
    0xA9CCE3, // blank/background 0 sky blue
    0x5D6D7E, // 1 string steel
    0x95A5A6, // 2  string isolater grey
    0xD35400, // 3 dreg orange
    0x58D68D, // 4 res lime
    0x1B2631, // 5 wall black
    0x641E16, // 6 rememberer burnt red
    0xD4AC0D, // 7 maker, dark yellow
    0x145A32, // 8 box, dark green
    0x9B59B6, // 9 string copier, purple
    0x154360, // 10 down dropper, dark navy
    0xE5E7E9, // 11 short lived, almost white
    0x4A235A, // 12 faller, dark purple
    0xE74C3C, // 13 strings splitter, bright red
    0x5DADE2, // 14 riser, blue
};

char cell_names[265][51] = {"BLANK", "STRING", ISOLATER", "DREG"...};

'''

# then all of the function definitions, then the function array.

'''
update_function_type update_functions[256] = {
    dont_update,
    update_string,
    update_isolater,
    update_dreg,
    update_res,
    dont_update,
    update_rememberer,
    update_something_maker,
    update_box,
    update_string_copier,
    update_downdropper,
    update_short_lived,
    update_faller,
    update_strings_splitter,
    update_riser,
    // the rest should be void
};
'''

# to define a new cell type, make a file `whatever.be.c` and define its behavior:
'''
#CELL WHATEVER 0xff00ff
void update_whatver(cell_t* cell) {
    // do some stuff
}
'''
# It's important that the #CELL line is right before the function definition and that the only
# thing before the name of the functions is "void" and an open parenthesis comes after it.
# If the color is ommitted, a random one based on the hash of the uppercase name will be used.
def name_to_color(name):
    random.seed(name)
    return '0x' + ''.join([random.choice('0123456789ABCDEF') for j in range(6)])

def multi_replace(string, orig_new_pairs):
    for (orig, new) in orig_new_pairs:
        string = string.replace(orig, new)
    return string

behave_help_string = \
'''
This file has the following tags:
        #CELL_ENUM
        #CELL_COLORS
        #CELL_NAMES
        #UPDATE_FUNCTIONS
which will be replaced with code generated based on the the cell be.c files
'''


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Combine your .be.c files with behave.c.')
    parser.add_argument('behave', metavar='B', type=str, 
            help=behave_help_string)

    parser.add_argument('cellfiles', action='append', nargs='+',
            help='Your .be.c files with #CELL (name) (color) tags.')

    args = parser.parse_args()

    behave_main = open(args.behave).read()
    lines = []
    for filename in args.cellfiles[0]:
        lines += open(filename).readlines() 

    names = []
    colors = []
    funs = []
    last_line_was_cell = False
    keeper_lines = []
    for line in lines:
        if not line.startswith('#CELL'):
            keeper_lines.append(line)
        if line.startswith('#CELL'):
            # match the name and optionall the color hex
            cell_search = re.search(r'#CELL\s+(\w*)\s+(0x[0-9A-Fa-f]{6})?.*', line)
            if cell_search:
                last_line_was_cell = True
                name = cell_search.group(1)
                names.append(name)
                maybe_color = cell_search.group(2)
                if maybe_color == None:
                    colors.append(name_to_color(name))
                else:
                    colors.append(maybe_color)
        elif last_line_was_cell:
            # match the name of the functions
            last_line_was_cell = False
            fun_search = re.search(r'void\s*(\w*)\s*\(', line)
            if fun_search:
                fun_name = fun_search.group(1)
                funs.append(fun_name)
            else:
                raise Exception("function name missing here")

    # make sure there weren't any weird occurrenes
    assert len(names) == len(colors) == len(funs)

    # other parts of the program depend on these
    assert names[0] == 'BLANK'
    assert names[1] == 'STRING'

    # names have to be less than 50 characters
    for name in names:
        assert len(name) < 50

    enum_string = 'enum CELL_TYPES {' \
            + ', '.join([n if i>1 else n+f'={i}' for i, n in enumerate(names)]) + '};\n'
    colors_string = 'unsigned int cell_colors[256] = {' + ', '.join(colors) + '};\n'
    names_string = 'char cell_names[265][51] = {' \
            + ', '.join(['"'+n+'"' for n in names]) + '};\n'
    funs_string = 'update_function_type update_functions[256] = {' + ', '.join(funs) + '};\n'

    function_definitions = '\n' + ''.join(keeper_lines) + '\n'

    behave_main = multi_replace(behave_main, [('#CELL_ENUM', enum_string),
        ('#CELL_COLORS', colors_string), ('#CELL_NAMES', names_string),
        ('#UPDATE_FUNCTIONS', function_definitions + funs_string)])
    behave_main = ascii_art.convert(behave_main)
    print(behave_main)
