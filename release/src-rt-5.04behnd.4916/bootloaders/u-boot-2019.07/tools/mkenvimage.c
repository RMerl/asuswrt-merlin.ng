// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 Free Electrons
 * David Wagner <david.wagner@free-electrons.com>
 *
 * Inspired from envcrc.c:
 * (C) Copyright 2001
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "compiler.h"
#include <u-boot/crc.h>
#include <version.h>

#define CRC_SIZE sizeof(uint32_t)
#define UBOOT_ENV_MAGIC  (0x75456e76)


static void usage(const char *exec_name)
{
	fprintf(stderr, "%s [-h] [-r] [-b] [-p <byte>] -s <environment partition size> -o <output> <input file>\n"
	       "\n"
	       "This tool takes a key=value input file (same as would a `printenv' show) and generates the corresponding environment image, ready to be flashed.\n"
	       "\n"
	       "\tThe input file is in format:\n"
	       "\t\tkey1=value1\n"
	       "\t\tkey2=value2\n"
	       "\t\t...\n"
	       "\tEmpty lines are skipped, and lines with a # in the first\n"
	       "\tcolumn are treated as comments (also skipped).\n"
	       "\t-r : the environment has multiple copies in flash\n"
	       "\t-b : the target is big endian (default is little endian)\n"
	       "\t--bootmagic : Prepend headers for boot_magic environment\n"
	       "\t--metadata : Prepend headers for Metadata\n"
	       "\t-p <byte> : fill the image with <byte> bytes instead of 0xff bytes\n"
	       "\t-V : print version information and exit\n"
	       "\n"
	       "If the input file is \"-\", data is read from standard input\n",
	       exec_name);
}

static int bootmagic = 0;
static int metadata = 0;

static struct option long_opts[] = {
	{"bootmagic",0,&bootmagic,1},
	{"metadata",0,&metadata,1},
	{0,0,0,0},
};

long int xstrtol(const char *s)
{
	long int tmp;

	errno = 0;
	tmp = strtol(s, NULL, 0);
	if (!errno)
		return tmp;

	if (errno == ERANGE)
		fprintf(stderr, "Bad integer format: %s\n",  s);
	else
		fprintf(stderr, "Error while parsing %s: %s\n", s,
				strerror(errno));

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	uint32_t crc, targetendian_crc;
	const char *txt_filename = NULL, *bin_filename = NULL;
	int txt_fd, bin_fd;
	unsigned char *dataptr, *envptr;
	unsigned char *filebuf = NULL;
	unsigned int filesize = 0, envsize = 0, datasize = 0;
	int bigendian = 0;
	int redundant = 0;
	unsigned char padbyte = 0xff;

	int option;
	int ret = EXIT_SUCCESS;

	struct stat txt_file_stat;

	int fp, ep;
	const char *prg;

	prg = basename(argv[0]);

	/* Turn off getopt()'s internal error message */
	opterr = 0;

	/* Parse the cmdline */
	while ((option = getopt_long(argc, argv, ":s:o:rbp:hV",long_opts,NULL)) != -1) {
		switch (option) {
		case 's':
			datasize = xstrtol(optarg);
			break;
		case 'o':
			bin_filename = strdup(optarg);
			if (!bin_filename) {
				fprintf(stderr, "Can't strdup() the output filename\n");
				return EXIT_FAILURE;
			}
			break;
		case 'r':
			redundant = 1;
			break;
		case 'b':
			bigendian = 1;
			break;
		case 'p':
			padbyte = xstrtol(optarg);
			break;
		case 'h':
			usage(prg);
			return EXIT_SUCCESS;
		case 'V':
			printf("%s version %s\n", prg, PLAIN_VERSION);
			return EXIT_SUCCESS;
		case ':':
			fprintf(stderr, "Missing argument for option -%c\n",
				optopt);
			usage(prg);
			return EXIT_FAILURE;
		case 0:
			break;
		default:
			fprintf(stderr, "Wrong option -%c\n", optopt);
			usage(prg);
			return EXIT_FAILURE;
		}
	}

	/* Check datasize and allocate the data */
	if (datasize == 0) {
		fprintf(stderr, "Please specify the size of the environment partition.\n");
		usage(prg);
		return EXIT_FAILURE;
	}

	dataptr = malloc(datasize * sizeof(*dataptr));
	if (!dataptr) {
		fprintf(stderr, "Can't alloc %d bytes for dataptr.\n",
				datasize);
		return EXIT_FAILURE;
	}

	/*
	 * envptr points to the beginning of the actual environment (after the
	 * crc and possible `redundant' byte
	 */
	envsize = datasize - (CRC_SIZE + redundant);
	envptr = dataptr + CRC_SIZE + redundant;

	/* Pad the environment with the padding byte */
	memset(envptr, padbyte, envsize);

	/* Open the input file ... */
	if (optind >= argc || strcmp(argv[optind], "-") == 0) {
		int readbytes = 0;
		int readlen = sizeof(*envptr) * 4096;
		txt_fd = STDIN_FILENO;

		do {
			filebuf = realloc(filebuf, filesize + readlen);
			if (!filebuf) {
				fprintf(stderr, "Can't realloc memory for the input file buffer\n");
				return EXIT_FAILURE;
			}
			readbytes = read(txt_fd, filebuf + filesize, readlen);
			if (readbytes < 0) {
				fprintf(stderr, "Error while reading stdin: %s\n",
						strerror(errno));
				return EXIT_FAILURE;
			}
			filesize += readbytes;
		} while (readbytes == readlen);

	} else {
		txt_filename = argv[optind];
		txt_fd = open(txt_filename, O_RDONLY);
		if (txt_fd == -1) {
			fprintf(stderr, "Can't open \"%s\": %s\n",
					txt_filename, strerror(errno));
			return EXIT_FAILURE;
		}
		/* ... and check it */
		ret = fstat(txt_fd, &txt_file_stat);
		if (ret == -1) {
			fprintf(stderr, "Can't stat() on \"%s\": %s\n",
					txt_filename, strerror(errno));
			return EXIT_FAILURE;
		}

		filesize = txt_file_stat.st_size;

		filebuf = mmap(NULL, sizeof(*envptr) * filesize, PROT_READ,
			       MAP_PRIVATE, txt_fd, 0);
		if (filebuf == MAP_FAILED) {
			fprintf(stderr, "mmap (%zu bytes) failed: %s\n",
					sizeof(*envptr) * filesize,
					strerror(errno));
			fprintf(stderr, "Falling back to read()\n");

			filebuf = malloc(sizeof(*envptr) * filesize);
			ret = read(txt_fd, filebuf, sizeof(*envptr) * filesize);
			if (ret != sizeof(*envptr) * filesize) {
				fprintf(stderr, "Can't read the whole input file (%zu bytes): %s\n",
					sizeof(*envptr) * filesize,
					strerror(errno));

				return EXIT_FAILURE;
			}
		}
		ret = close(txt_fd);
	}

	/* Parse a byte at time until reaching the file OR until the environment fills
	 * up. Check ep against envsize - 1 to allow for extra trailing '\0'. */
	for (fp = 0, ep = 0 ; fp < filesize && ep < envsize - 1; fp++) {
		if (filebuf[fp] == '\n') {
			if (fp == 0 || filebuf[fp-1] == '\n') {
				/*
				 * Skip empty lines.
				 */
				continue;
			} else if (filebuf[fp-1] == '\\') {
				/*
				 * Embedded newline in a variable.
				 *
				 * The backslash was added to the envptr; rewind
				 * and replace it with a newline
				 */
				ep--;
				envptr[ep++] = '\n';
			} else {
				/* End of a variable */
				envptr[ep++] = '\0';
			}
		} else if ((fp == 0 || filebuf[fp-1] == '\n') && filebuf[fp] == '#') {
			/* Comment, skip the line. */
			while (++fp < filesize && filebuf[fp] != '\n')
			continue;
		} else {
			envptr[ep++] = filebuf[fp];
		}
	}
	/* If there are more bytes in the file still, it means the env filled up
	 * before parsing the whole file.  Eat comments & whitespace here to see if
	 * there was anything meaning full left in the file, and if so, throw a error
	 * and exit. */
	for( ; fp < filesize; fp++ )
	{
		if (filebuf[fp] == '\n') {
			if (fp == 0 || filebuf[fp-1] == '\n') {
				/* Ignore blank lines */
				continue;
			}
		} else if ((fp == 0 || filebuf[fp-1] == '\n') && filebuf[fp] == '#') {
			while (++fp < filesize && filebuf[fp] != '\n')
			continue;
		} else {
			fprintf(stderr, "The environment file is too large for the target environment storage\n");
			return EXIT_FAILURE;
		}
	}
	/*
	 * Make sure there is a final '\0'
	 * And do it again on the next byte to mark the end of the environment.
	 */
	if (envptr[ep-1] != '\0') {
		envptr[ep++] = '\0';
		/*
		 * The text file doesn't have an ending newline.  We need to
		 * check the env size again to make sure we have room for two \0
		 */
		if (ep >= envsize) {
			fprintf(stderr, "The environment file is too large for the target environment storage\n");
			return EXIT_FAILURE;
		}
		envptr[ep] = '\0';
	} else {
		envptr[ep] = '\0';
	}

	/* Computes the CRC and put it at the beginning of the data */
	crc = crc32(0, envptr, envsize);
	targetendian_crc = bigendian ? cpu_to_be32(crc) : cpu_to_le32(crc);

	memcpy(dataptr, &targetendian_crc, sizeof(targetendian_crc));
	if (redundant)
		dataptr[sizeof(targetendian_crc)] = 1;

	if (!bin_filename || strcmp(bin_filename, "-") == 0) {
		bin_fd = STDOUT_FILENO;
	} else {
		bin_fd = creat(bin_filename, S_IRUSR | S_IWUSR | S_IRGRP |
					     S_IWGRP);
		if (bin_fd == -1) {
			fprintf(stderr, "Can't open output file \"%s\": %s\n",
					bin_filename, strerror(errno));
			return EXIT_FAILURE;
		}
	}

	if (bootmagic || metadata) {
		uint32_t magichdr[2];
		magichdr[0] = bigendian ? cpu_to_be32(UBOOT_ENV_MAGIC) : cpu_to_le32(UBOOT_ENV_MAGIC);
		magichdr[1] = bigendian ? cpu_to_be32(datasize) : cpu_to_le32(datasize);
		if (metadata) {
			magichdr[0] = magichdr[1]; 
		}
		if (write(bin_fd, magichdr, sizeof(magichdr)) !=
				sizeof(magichdr)) {
			fprintf(stderr, "write() failed: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	
	if (write(bin_fd, dataptr, sizeof(*dataptr) * datasize) !=
			sizeof(*dataptr) * datasize) {
		fprintf(stderr, "write() failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	ret = close(bin_fd);

	return ret;
}
