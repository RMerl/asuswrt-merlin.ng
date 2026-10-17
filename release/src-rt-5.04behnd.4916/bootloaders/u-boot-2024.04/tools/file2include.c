// SPDX-License-Identifier: GPL-2.0+
/*
 * Convert a file image to a C define
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * For testing EFI disk management we need an in memory image of
 * a disk.
 *
 * The tool file2include converts a file to a C include. The file
 * is separated into strings of 8 bytes. Only the non-zero strings
 * are written to the include. The output format has been designed
 * to maintain readability.
 *
 * As the disk image needed for testing contains mostly zeroes a high
 * compression ratio can be attained.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Size of the blocks written to the compressed file */
#define BLOCK_SIZE 8

int main(int argc, char *argv[])
{
	FILE *file;
	int ret;
	unsigned char *buf;
	size_t count, i, j;

	/* Provide usage help */
	if (argc != 2) {
		printf("Usage:\n%s FILENAME\n", argv[0]);
		return EXIT_FAILURE;
	}
	/* Open file */
	file = fopen(argv[1], "r");
	if (!file) {
		perror("fopen");
		return EXIT_FAILURE;
	}
	/* Get file length */
	ret = fseek(file, 0, SEEK_END);
	if (ret < 0) {
		perror("fseek");
		return EXIT_FAILURE;
	}
	count = ftell(file);
	if (!count) {
		fprintf(stderr, "File %s has length 0\n", argv[1]);
		return EXIT_FAILURE;
	}
	rewind(file);
	/* Read file */
	buf = malloc(count);
	if (!buf) {
		perror("calloc");
		return EXIT_FAILURE;
	}
	count = fread(buf, 1, count, file);

	/* Generate output */
	printf("/* SPDX-License-Identifier: GPL-2.0+ */\n");
	printf("/*\n");
	printf(" *  Non-zero %u byte strings of a disk image\n", BLOCK_SIZE);
	printf(" *\n");
	printf(" *  Generated with tools/file2include\n");
	printf(" */\n\n");
	printf("#define EFI_ST_DISK_IMG { 0x%08zx, { \\\n", count);

	for (i = 0; i < count; i += BLOCK_SIZE) {
		int c = 0;

		for (j = i; j < i + BLOCK_SIZE && j < count; ++j) {
			if (buf[j])
				c = 1;
		}
		if (!c)
			continue;
		printf("\t{0x%08zx, \"", i);
		for (j = i; j < i + BLOCK_SIZE && j < count; ++j)
			printf("\\x%02x", buf[j]);
		printf("\"}, /* ");
		for (j = i; j < i + BLOCK_SIZE && j < count; ++j) {
			if (buf[j] != '*' && buf[j] >= 0x20 && buf[j] <= 0x7e)
				printf("%c", buf[j]);
			else
				printf(".");
		}
		printf(" */ \\\n");
	}
	printf("\t{0, NULL} } }\n");

	/* Release resources */
	free(buf);
	ret = fclose(file);
	if (ret) {
		perror("fclose");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
