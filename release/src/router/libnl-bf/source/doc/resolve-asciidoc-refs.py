#!/usr/bin/env python

import fileinput
import re
import sys

refs = {}
complete_file = ""

for line in open(sys.argv[1], 'r'):
	complete_file += line

for m in re.findall('\[\[(.+)\]\]\n=+ ([^\n]+)', complete_file):
	ref, title = m
	refs["<<" + ref + ">>"] = "<<" + ref + ", " + title + ">>"

def translate(match):
	try:
		return refs[match.group(0)]
	except KeyError:
		return ""

rc = re.compile('|'.join(map(re.escape, sorted(refs, reverse=True))))
for line in open(sys.argv[1], 'r'):
	print rc.sub(translate, line),
