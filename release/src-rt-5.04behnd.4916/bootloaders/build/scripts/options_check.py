#!/bin/python

import os
import sys

def PRINT (a):
	return

#open file
file="options_help.txt"

if len(sys.argv) > 1:
	file=sys.argv[1] 

F=open(file)

#read all lines
if F:
	lines=F.readlines()


in_option = False
in_help = False

for l in lines:
# check if its a comment
	l=l.strip()
	if l.startswith("#") == True :
		PRINT("Comment " + l);
		continue
# check if its an option
	if l.startswith("option:") == True :
		PRINT("option " + l);
		in_option=True
# check if its empty line
	if len(l) == 0 :
		PRINT("empty line")
		in_option = False
		in_help = False
	#look for help
	if len(l) > 0 :
		PRINT("help " + l);
		if in_option == True :
			in_help=True
		else :
			print("malformed option help " + l)
