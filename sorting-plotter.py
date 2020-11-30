from matplotlib import pyplot as plt
import sys
plt.style.use('ggplot')


def multiscale_mse(a, b, rangea, rangeb):
    return ((a/rangea)-(b/rangeb))**2

def spots_wrong_by(a, b, rangea, rangeb):
    return abs(a - b*(rangea/rangeb))

def positional_error(value, pos, range_value, range_pos):
    return abs(value*(range_pos/range_value) - pos)

sumlist = []

max_pos = 0
for line in open(sys.argv[2]):
    value, pos = (int(x.strip()) for x in line.split(', '))
    if pos == 255:
        continue
    if pos > max_pos:
        max_pos = pos
max_pos += 1
print(max_pos)

for csv in sys.argv[2:]:
    sums = []
    cursum = 0
    num_in_cur = 0
    breaker = 100
    for line in open(csv):
        value, pos = (int(x.strip()) for x in line.split(', '))
        if pos == 255:
            continue
        if pos > max_pos:
            max_pos = pos
        cursum += positional_error(value, pos, 256, max_pos)
        num_in_cur += 1
        if num_in_cur == breaker:
            sums.append(cursum/breaker)
            cursum = 0
            num_in_cur = 0
    sumlist.append(sums)

minlen = min([len(x) for x in sumlist])
plt.ylim(bottom=0,top=30)
#plt.plot(sums, c='grey')
handles = []
for s, label in zip(sumlist, ['0%', '25%', '50%', '75%', '100%']):
    handles.append(plt.plot(s[:minlen], label=label)[0])
plt.legend(handles=handles[::-1])
plt.xlabel('Time')
plt.ylabel('Positional Error')
plt.title(sys.argv[1])

plt.hlines(max_pos/3, 0, minlen, colors='black')
plt.show()
