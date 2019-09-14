/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: my_getopt.h,v 1.1.1.1.2.3 2004/02/02 08:06:24 sla Exp $
*/

#ifndef _MY_GETOPT_H_
#define _MY_GETOPT_H_

#define MY_GETOPT_ARG_REQUIRED 0x1
#define MY_GETOPT_REQUIRED 0x2
#define MY_GETOPT_ALLOW_REPEAT 0x4

#define MY_GETOPT_MAX_OPTSTR 4096

struct getopt_parms {
	char name;
	int flag;
	int count;
	char *arg;
};

int my_getopt(int argc, char * const argv[], struct getopt_parms parms[]);

#endif
