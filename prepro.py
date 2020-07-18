import itertools
import re
import sys

def ascii_to_grid(string):
    string = string.replace('|', '')
    letters = []
    lines = string.split('\n')
    special_line = lines[0]
    lines = lines[1:]
    for i, line in enumerate(lines):
        for j, c in enumerate(line):
            if c == '.':
                center_i = i
                center_j = j
            elif c != ' ':
                letters.append((i, j, c))
    out = []
    for i, j, c in letters:
        out.append((i-center_i, j-center_j, c))
    return out, special_line

def rotate_letters(letters, rotation):
    ''' rotation should be 0 to 4 '''
    out = []
    for (i, j, c) in letters:
        if rotation == 0:
            out.append((i, j, c))
        elif rotation == 1:
            out.append((j, -i, c))
        elif rotation == 2:
            out.append((-i, -j, c))
        elif rotation == 3:
            out.append((-j, i, c))
        else:
            raise Exception()
    return out


def pos_part(n):
    if n > 0:
        return n
    return 0

def neg_part(n):
    if n < 0:
        return abs(n)
    return 0

def chainer(di, dj):
    return ''.join(['->right'*pos_part(dj), 
                    '->left'*neg_part(dj),
                    '->up'*neg_part(di),
                    '->down'*pos_part(di)])


#for (i, j, c) in ascii_to_grid(example):
#    print(f'cell_t {c} = cell{chainer(i, j)}')

def sub_between(string, starter, ender, subfun):
    parts = string.split(starter)
    out = []
    out.append(parts[0])
    for part in parts[1:]:
        stuff = part.split(ender, 1)
        if len(stuff) == 2:
            out.append(subfun(stuff[0])+stuff[1])
        else:
            raise Exception("oh no your strings are all wrong", stuff)
    return ''.join(out)

def third(ls):
    return ls[2]

def grid_to_code(letters):
    code = []
    declarations = []
    for _, group_iter in itertools.groupby(sorted(letters,key=third), key=third):
        group = list(group_iter)
        if len(group) == 1:
            (i, j, c) = group[0]
            declarations.append(f'cell_t* {c};')
            code.append(f'{c} = cell{chainer(i, j)};')
        else:
            var_name = group[0][2]
            declarations.append(f'cell_t* {var_name};')
            code.append(f'switch(rand() % {len(group)})' + ' {')
            for (n, (i, j, c)) in enumerate(group):
                code.append(f'\tcase {n}: {c} = cell{chainer(i, j)}; break;')
            code.append('}')
    return code, declarations

def process_ascii_art(string):
    declarations = []
    code = []
    
    letters, special_line = ascii_to_grid(string)



    if 'ROTATE' in special_line:
        rotater_code = special_line.split('ROTATE')[1].strip()
        code.append('// Rotating')
        code.append(f'switch(({rotater_code})%4) ' + '{')
        for i in range(4):
            code.append(f'case {i}:')
            more_code, declarations = grid_to_code(rotate_letters(letters, i))
            code += more_code
            code.append(f'break;')
        code.append('}')

    else:
        code, declarations = grid_to_code(letters)

    return '\n'.join(declarations + code)


if __name__ == "__main__":
    source = open(sys.argv[1]).read()
    print(sub_between(source, 'PATTERN', ';', process_ascii_art))
