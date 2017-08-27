#!/usr/bin/python

import sys

W = {}
W['A'] = []
W['W'] = []
W['F'] = W['W']

for line in sys.stdin:
	i = line.find ('#')
	if i >= 0:
		line = line[:i]
	line = line.strip ()
	if not len (line):
		continue

	fields = [x.strip () for x in line.split (';')]
	chars = fields[0]
	width = fields[1]

	if width not in ['A', 'W', 'F']:
		continue

	if chars.find ('..') > 0:
		(start,end) = chars.split ('..')
	else:
		start = chars
		end = chars
	start, end = int(start,16), int(end,16)

	for i in range (start, end+1):
		W[width].append (i)


def write_intervals (S):
	S.sort ()
	start = S[0];
	end = start - 1
	for c in S:
		if c == end+1:
			end += 1
			continue
		else:
			print "{0x%04X, 0x%04X}, " % (start, end)
			start = c
			end = start
	print "{0x%04X, 0x%04X} " % (start, end)



print "table for g_unichar_iswide():"
print
write_intervals (W['W'])
print
print "table for g_unichar_iswide_cjk():"
print
write_intervals (W['A'])
