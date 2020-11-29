import colorsys
print('static const unsigned int data_colors[] = {')
def cap(x, c):
    if x >= c:
        return c-1
    return x
def hue_printer(h):
    g, b, r = [cap(int(x*256), 256) for x in colorsys.hsv_to_rgb(h/256, 1, 1)]
    hexx = '0x%02X%02X%02X' % (r, g, b)
    print(f'    {hexx},')

for h in range(265):
    if h == 0:
        print('    0x000000,')
    elif h == 1:
        print('    0xff0000,')
    elif h == 2:
        print('    0x00ff00,')
    elif h == 3:
        print('    0x0000ff,')
    else:
        hue_printer(h*.8)
print('};')
