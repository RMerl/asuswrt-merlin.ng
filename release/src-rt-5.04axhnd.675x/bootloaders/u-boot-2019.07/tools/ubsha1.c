// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, <hs@denx.de>
 */

#include "os_support.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <u-boot/sha1.h>

int main (int argc, char **argv)
{
	unsigned char output[20];
	int i, len;

	char	*imagefile;
	char	*cmdname = *argv;
	unsigned char	*ptr;
	unsigned char	*data;
	struct stat sbuf;
	unsigned char	*ptroff;
	int	ifd;
	int	off;

	if (argc > 1) {
		imagefile = argv[1];
		ifd = open (imagefile, O_RDWR|O_BINARY);
		if (ifd < 0) {
			fprintf (stderr, "%s: Can't open %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}
		if (fstat (ifd, &sbuf) < 0) {
			fprintf (stderr, "%s: Can't stat %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}
		len = sbuf.st_size;
		ptr = (unsigned char *)mmap(0, len,
				    PROT_READ, MAP_SHARED, ifd, 0);
		if (ptr == (unsigned char *)MAP_FAILED) {
			fprintf (stderr, "%s: Can't read %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		/* create a copy, so we can blank out the sha1 sum */
		data = malloc (len);
		memcpy (data, ptr, len);
		off = SHA1_SUM_POS;
		ptroff = &data[len +  off];
		for (i = 0; i < SHA1_SUM_LEN; i++) {
			ptroff[i] = 0;
		}

		sha1_csum ((unsigned char *) data, len, (unsigned char *)output);

		printf ("U-Boot sum:\n");
		for (i = 0; i < 20 ; i++) {
		    printf ("%02X ", output[i]);
		}
		printf ("\n");
		/* overwrite the sum in the bin file, with the actual */
		lseek (ifd, SHA1_SUM_POS, SEEK_END);
		if (write (ifd, output, SHA1_SUM_LEN) != SHA1_SUM_LEN) {
			fprintf (stderr, "%s: Can't write %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		free (data);
		(void) munmap((void *)ptr, len);
		(void) close (ifd);
	}

	return EXIT_SUCCESS;
}
