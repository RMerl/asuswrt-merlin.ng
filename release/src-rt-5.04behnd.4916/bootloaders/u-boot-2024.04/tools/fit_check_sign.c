// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * DENX Software Engineering
 * Heiko Schocher <hs@denx.de>
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
	fprintf(stderr, "Usage: %s -f fit file -k key file -c config name\n"
			 "          -f ==> set fit file which should be checked'\n"
			 "          -k ==> set key .dtb file which contains the key'\n"
			 "          -c ==> set the configuration name'\n",
		cmdname);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int ffd = -1;
	int kfd = -1;
	struct stat fsbuf;
	struct stat ksbuf;
	void *fit_blob;
	char *fdtfile = NULL;
	char *keyfile = NULL;
	char *config_name = NULL;
	char cmdname[256];
	int ret;
	void *key_blob;
	int c;

	strncpy(cmdname, *argv, sizeof(cmdname) - 1);
	cmdname[sizeof(cmdname) - 1] = '\0';
	while ((c = getopt(argc, argv, "f:k:c:")) != -1)
		switch (c) {
		case 'f':
			fdtfile = optarg;
			break;
		case 'k':
			keyfile = optarg;
			break;
		case 'c':
			config_name = optarg;
			break;
		default:
			usage(cmdname);
			break;
	}

	if (!fdtfile) {
		fprintf(stderr, "%s: Missing fdt file\n", *argv);
		usage(*argv);
	}
	if (!keyfile) {
		fprintf(stderr, "%s: Missing key file\n", *argv);
		usage(*argv);
	}

	ffd = mmap_fdt(cmdname, fdtfile, 0, &fit_blob, &fsbuf, false, true);
	if (ffd < 0)
		return EXIT_FAILURE;
	kfd = mmap_fdt(cmdname, keyfile, 0, &key_blob, &ksbuf, false, true);
	if (kfd < 0)
		return EXIT_FAILURE;

	image_set_host_blob(key_blob);
	ret = fit_check_sign(fit_blob, key_blob, config_name);
	if (!ret) {
		ret = EXIT_SUCCESS;
		fprintf(stderr, "Signature check OK\n");
	} else {
		ret = EXIT_FAILURE;
		fprintf(stderr, "Signature check bad (error %d)\n", ret);
	}

	(void) munmap((void *)fit_blob, fsbuf.st_size);
	(void) munmap((void *)key_blob, ksbuf.st_size);

	close(ffd);
	close(kfd);
	exit(ret);
}
