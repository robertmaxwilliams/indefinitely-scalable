import itertools
import re
import sys

def ascii_to_grid(string):
    string = string.replace('|', '')
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
    out.append(parts[0])
    for part in parts[1:]:
        stuff = part.split(ender, 1)
        if len(stuff) == 2:
            out.append(subfun(stuff[0])+stuff[1])
        else:
            raise Exception("oh no your strings are all wrong", stuff)
    return ''.join(out)

def process_ascii_art(string):
    out = []
    
    letters = ascii_to_grid(string)

    def third(ls):
        return ls[2]
    for _, group_iter in itertools.groupby(sorted(letters,key=third), key=third):
        group = list(group_iter)
        if len(group) == 1:
            (i, j, c) = group[0]
            out.append(f'cell_t* {c} = cell{chainer(i, j)};\n')
        else:
            var_name = group[0][2]
            out.append(f'cell_t* {var_name};\n')
            out.append(f'switch(rand() % {len(group)})' + ' {\n')
            for (n, (i, j, c)) in enumerate(group):
                out.append(f'\tcase {n}: {c} = cell{chainer(i, j)}; break;\n')
            out.append('}\n')
    return ''.join(out)


if __name__ == "__main__":
    source = open(sys.argv[1]).read()
    print(sub_between(source, 'PATTERN', ';', process_ascii_art))
