// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * DENX Software Engineering
 * Heiko Schocher <hs@denx.de>
 *
 * fit_info: print the offset and the len of a property from
 *	     node in a fit file.
 *
 * Based on:
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 *
 * Updated-by: Prafulla Wadaskar <prafulla@marvell.com>
 *		FIT image specific code abstracted from mkimage.c
 *		some functions added to address abstraction
 *
 * All rights reserved.
 */

#include "mkimage.h"
#include "fit_common.h"
#include <image.h>
#include <u-boot/crc.h>

void usage(char *cmdname)
{
	fprintf(stderr, "Usage: %s -f fit file -n node -p property\n"
			 "          -f ==> set fit file which is used'\n"
			 "          -n ==> set node name'\n"
			 "          -p ==> set property name'\n",
		cmdname);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int ffd = -1;
	struct stat fsbuf;
	void *fit_blob;
	int len;
	int  nodeoffset;	/* node offset from libfdt */
	const void *nodep;	/* property node pointer */
	char *fdtfile = NULL;
	char *nodename = NULL;
	char *propertyname = NULL;
	char cmdname[256];
	int c;

	strncpy(cmdname, *argv, sizeof(cmdname) - 1);
	cmdname[sizeof(cmdname) - 1] = '\0';
	while ((c = getopt(argc, argv, "f:n:p:")) != -1)
		switch (c) {
		case 'f':
			fdtfile = optarg;
			break;
		case 'n':
			nodename = optarg;
			break;
		case 'p':
			propertyname = optarg;
			break;
		default:
			usage(cmdname);
			break;
		}

	if (!fdtfile) {
		fprintf(stderr, "%s: Missing fdt file\n", *argv);
		usage(*argv);
	}
	if (!nodename) {
		fprintf(stderr, "%s: Missing node name\n", *argv);
		usage(*argv);
	}
	if (!propertyname) {
		fprintf(stderr, "%s: Missing property name\n", *argv);
		usage(*argv);
	}
	ffd = mmap_fdt(cmdname, fdtfile, 0, &fit_blob, &fsbuf, false, false);

	if (ffd < 0) {
		printf("Could not open %s\n", fdtfile);
		exit(EXIT_FAILURE);
	}

	nodeoffset = fdt_path_offset(fit_blob, nodename);
	if (nodeoffset < 0) {
		printf("%s not found.", nodename);
		exit(EXIT_FAILURE);
	}
	nodep = fdt_getprop(fit_blob, nodeoffset, propertyname, &len);
	if (len == 0) {
		printf("len == 0 %s\n", propertyname);
		exit(EXIT_FAILURE);
	}

	printf("NAME: %s\n", fit_get_name(fit_blob, nodeoffset, NULL));
	printf("LEN: %d\n", len);
	printf("OFF: %d\n", (int)(nodep - fit_blob));
	(void) munmap((void *)fit_blob, fsbuf.st_size);

	close(ffd);
	exit(EXIT_SUCCESS);
}
