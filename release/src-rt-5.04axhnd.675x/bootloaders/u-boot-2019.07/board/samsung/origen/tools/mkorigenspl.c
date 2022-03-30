// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Samsung Electronics
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define BUFSIZE			(16*1024)
#define IMG_SIZE		(16*1024)
#define SPL_HEADER_SIZE		16
#define FILE_PERM		(S_IRUSR | S_IWUSR | S_IRGRP \
				| S_IWGRP | S_IROTH | S_IWOTH)
#define SPL_HEADER		"S5PC210 HEADER  "
/*
* Requirement:
* IROM code reads first 14K bytes from boot device.
* It then calculates the checksum of 14K-4 bytes and compare with data at
* 14K-4 offset.
*
* This function takes two filenames:
* IN  "u-boot-spl.bin" and
* OUT "$(BOARD)-spl.bin as filenames.
* It reads the "u-boot-spl.bin" in 16K buffer.
* It calculates checksum of 14K-4 Bytes and stores at 14K-4 offset in buffer.
* It writes the buffer to "$(BOARD)-spl.bin" file.
*/

int main(int argc, char **argv)
{
	int i, len;
	unsigned char buffer[BUFSIZE] = {0};
	int ifd, ofd;
	unsigned int checksum = 0, count;

	if (argc != 3) {
		printf(" %d Wrong number of arguments\n", argc);
		exit(EXIT_FAILURE);
	}

	ifd = open(argv[1], O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			argv[0], argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	ofd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, FILE_PERM);
	if (ofd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			argv[0], argv[2], strerror(errno));
		if (ifd)
			close(ifd);
		exit(EXIT_FAILURE);
	}

	len = lseek(ifd, 0, SEEK_END);
	lseek(ifd, 0, SEEK_SET);

	memcpy(&buffer[0], SPL_HEADER, SPL_HEADER_SIZE);

	count = (len < (IMG_SIZE - SPL_HEADER_SIZE))
		? len : (IMG_SIZE - SPL_HEADER_SIZE);

	if (read(ifd, buffer + SPL_HEADER_SIZE, count) != count) {
		fprintf(stderr, "%s: Can't read %s: %s\n",
			argv[0], argv[1], strerror(errno));

		if (ifd)
			close(ifd);
		if (ofd)
			close(ofd);

		exit(EXIT_FAILURE);
	}

	for (i = 0; i < IMG_SIZE - SPL_HEADER_SIZE; i++)
		checksum += buffer[i+16];

	*(unsigned long *)buffer ^= 0x1f;
	*(unsigned long *)(buffer+4) ^= checksum;

	for (i = 1; i < SPL_HEADER_SIZE; i++)
		buffer[i] ^= buffer[i-1];

	if (write(ofd, buffer, BUFSIZE) != BUFSIZE) {
		fprintf(stderr, "%s: Can't write %s: %s\n",
			argv[0], argv[2], strerror(errno));

		if (ifd)
			close(ifd);
		if (ofd)
			close(ofd);

		exit(EXIT_FAILURE);
	}

	if (ifd)
		close(ifd);
	if (ofd)
		close(ofd);

	return EXIT_SUCCESS;
}
