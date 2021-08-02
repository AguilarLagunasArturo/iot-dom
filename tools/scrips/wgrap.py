import sys

file = ''
if not len(sys.argv) == 2:
	exit()

file = sys.argv[1]
with open(file, 'r') as f:
	lines = f.read().split('\n')

for l in lines:
	print('"{}"'.format(l))
