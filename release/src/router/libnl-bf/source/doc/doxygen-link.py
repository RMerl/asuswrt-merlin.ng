#!/usr/bin/env python

import fileinput
import re
import sys

links = {}

for line in open(sys.argv[1], 'r'):
	m = re.match('^([^=]+)=([^\n]+)$', line);
	if m:
		link = "<a href=\"" + m.group(2) + "\" class=\"dg\">" + m.group(1) + "</a>"
		links[m.group(1)] = link

def translate(match):
	return links[match.group(0)]

rc = re.compile('|'.join(map(re.escape, sorted(links, reverse=True))))
for line in open(sys.argv[2], 'r'):
	print rc.sub(translate, line),
