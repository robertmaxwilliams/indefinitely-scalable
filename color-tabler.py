import colorsys
print('const unsigned int data_colors[] = {')
def hue_printer(h):
    r, g, b = [int(x*256)%256 for x in colorsys.hsv_to_rgb(h/256, 0.5, 0.5)]
    hexx = '0x%02X%02X%02X' % (r, g, b)
    print(f'    {hexx},')

for h in range(265):
    if h == 0:
        print('    0x000000,')
    if h == 1:
        hue_printer(50)
    if h == 2:
        hue_printer(100)
    if h == 3:
        hue_printer(150)
    hue_printer(h)
print('};')
