/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: my_getopt.c,v 1.2.2.3 2004/02/02 08:06:24 sla Exp $
*/

#include <common.h>

/* getopt() */
#include <unistd.h>

/* fprintf() */
#include <stdio.h>

#include <my_getopt.h>

extern char *optarg;
extern int optind, opterr, optopt;

int my_getopt(int argc, char * const argv[],
	struct getopt_parms parms[])
{
	int c, i, p;
	static int flag = 0, my_opterr = 1;
	static char optstring[MY_GETOPT_MAX_OPTSTR];

	if (flag++ == 0) {
		p = 0; i = 0;
		while ((parms[i].name != 0) && (p < (MY_GETOPT_MAX_OPTSTR - 1))) {
			optstring[p++] = parms[i].name;
			if (parms[i].flag & MY_GETOPT_ARG_REQUIRED) optstring[p++] = ':';
			optstring[p] = 0;
			i++;
		}
		flag = 1;
	}

	c = getopt(argc, argv, optstring);
	switch (c) {
		case '?':
			if (my_opterr) fprintf(stderr, "Wrong parameters\n");
			break;

		case -1:
			flag = 0;
			i = 0;
			while (parms[i].name != 0) {
				if ((parms[i].flag & MY_GETOPT_REQUIRED) && (parms[i].count == 0)) {
					if (my_opterr) fprintf(stderr, "Missing required option\n");
					return '?';
				}
				i++;
			}
			break;

		default:
			i = 0;
			while ((parms[i].name != 0) && (parms[i].name != c)) {
				i++;
			}
			if (parms[i].flag & MY_GETOPT_ARG_REQUIRED) {
				if (optarg == 0) {
					if (my_opterr) fprintf(stderr, "Option `-%c': %s\n",
							c, "require parameter");
					return '?';
				} else parms[i].arg = optarg;
			}
			if ((++parms[i].count > 1) && !(parms[i].flag & MY_GETOPT_ALLOW_REPEAT)) {
				if (my_opterr) fprintf(stderr, "Option `-%c': %s\n",
						c, "repeat not allowed");
				return '?';
			}
			break;
	}
	return c;
}
