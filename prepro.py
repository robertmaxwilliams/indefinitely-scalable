import itertools
import re
import sys
def ascii_to_grid(string):
    letters = []
    lines = string.split('\n')
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
    for part in parts:
        stuff = part.split(ender)
        if len(stuff) == 1:
            out.append(part)
        elif len(stuff) == 2:
            out.append(subfun(stuff[0])+stuff[1])
        else:
            raise Exception("oh no your strings are all wrong", stuff)
    return ''.join(out)

def process_ascii_art(string):
    out = []
    for (i, j, c) in ascii_to_grid(string):
        out.append(f'cell_t* {c} = cell{chainer(i, j)};')
    return ''.join(out)

if __name__ == "__main__":
    source = open(sys.argv[1]).read()
    print(sub_between(source, '#PATTERN', '#ENDPATTERN', process_ascii_art))



    
