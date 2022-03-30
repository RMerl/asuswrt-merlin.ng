// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Broadcom
 *
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

char *exclude[] = {
	"env_boot_magic=",
	"stdin=",
	"stdout=",
	"stderr=",
	"fileaddr=",
	"filesize=",
	"ethact=",
	"partition=",
	"mtddevname=",
	"fdtcontroladdr=",
	NULL
};

static void usage(const char *exec_name)
{
	fprintf(stderr, "%s [-h] <input image file>  <input environment value file>\n"
	       "\n"
	       "This tool takes a key=value input file (same as would a `printenv' show) and replaces the environment boot_magic_env in an existing image with the new environment.\n"
	       "\n"
	       "\tThe input file is in format:\n"
	       "\t\tkey1=value1\n"
	       "\t\tkey2=value2\n"
	       "\t\t...\n"
	       "\tEmpty lines are skipped, and lines with a # in the first\n"
	       "\tcolumn are treated as comments (also skipped).\n"
	       "\t-V : print version information and exit\n"
	       "\n"
	       "If the input file is \"-\", data is read from standard input\n",
	       exec_name);
}


static struct option long_opts[] = {
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

static int check_excluded(char *evar, int max)
{
	int i = 0;
	int n;
	while (NULL != exclude[i]) {
		n = strlen(exclude[i]);
		if (0 == strncmp(evar, exclude[i], n < max ? n : max))
		{
			return(1);
		}
		i++;
	}
	return(0);
}

int main(int argc, char **argv)
{
	uint32_t crc, havecrc, targetendian_crc;
	const char *input_img_filename = NULL,*txt_filename = NULL;
	int input_img_fd, txt_fd;
	char *dataptr = NULL, *envptr;
	char *filebuf = NULL;
	unsigned int filesize = 0, envsize = 0, datasize = 0, tmpdatasize = 0;
	int bigendian = 0;
	unsigned char padbyte = 0xff;
	uint32_t header[3];
	int offsets[16];
	int found = 0;
	int i;

	int option;
	int ret = EXIT_SUCCESS;

	struct stat txt_file_stat;
	struct stat input_img_file_stat;
	unsigned img_filesize = 0;

	int fp, ep;
	const char *prg;

	prg = basename(argv[0]);

	/* Turn off getopt()'s internal error message */
	opterr = 0;

	/* Parse the cmdline */
	while ((option = getopt_long(argc, argv, "hV",long_opts,NULL)) != -1) {
		switch (option) {
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

	if (argc > optind) {
		input_img_filename = argv[optind];
		input_img_fd = open(input_img_filename, O_RDWR);
		ret = fstat(input_img_fd, &input_img_file_stat);
		if (ret == -1) {
			fprintf(stderr, "Can't stat() on \"%s\": %s\n",
					input_img_filename, strerror(errno));
			return EXIT_FAILURE;
		}

		img_filesize = input_img_file_stat.st_size;
	} else {
		fprintf(stderr,"finame is required\n");
		exit(1);
	}

	for (i = 0 ; i < img_filesize ; i += 4096) {
		lseek(input_img_fd, i, SEEK_SET);
		read(input_img_fd, header, sizeof(header));
		if ( UBOOT_ENV_MAGIC == header[0] ) {
			printf("found magic at %x size %d\n",i,header[1]);
		} else {
			continue;
		}
		tmpdatasize = header[1];
		if (tmpdatasize > 65536) {
			fprintf(stderr,"datasize > 64K not allowed\n");
			exit(1);
		}
		if (dataptr == NULL) {
			dataptr = malloc(tmpdatasize + 4);
		} else {
			dataptr = realloc(dataptr,tmpdatasize + 4);
		}
		lseek(input_img_fd, i+8, SEEK_SET);
		read(input_img_fd, dataptr, tmpdatasize + 4);
		crc = crc32(0, (const unsigned char *)dataptr+4, tmpdatasize-4);
		havecrc = header[2];
		if (crc == havecrc) {
			printf("CRC ok\n");
			offsets[found++] = i;
			datasize = tmpdatasize;
		} else {
			printf("CRC FAIL\n");
		}
	}

	/* Check datasize and allocate the data */
	if (datasize == 0) {
		fprintf(stderr, "No header found\n");
		return EXIT_FAILURE;
	}

	/*
	 * envptr points to the beginning of the actual environment (after the
	 * crc and possible `redundant' byte
	 */
	envsize = datasize - CRC_SIZE;
	envptr = dataptr + CRC_SIZE;

	/* Pad the environment with the padding byte */
	memset(envptr, padbyte, envsize);

	/* Open the input file ... */
	if (optind+1 < argc ) {
		txt_filename = argv[optind+1];
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
		} else if ((fp == 0 || filebuf[fp-1] == '\n') && (filebuf[fp] == '#' || check_excluded(&filebuf[fp],filesize-fp) != 0)) {
			/* Comment or excluded, skip the line. */
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
	crc = crc32(0, (const unsigned char *)envptr, envsize);
	targetendian_crc = bigendian ? cpu_to_be32(crc) : cpu_to_le32(crc);

	memcpy(dataptr, &targetendian_crc, sizeof(targetendian_crc));
	for (i = 0 ; i < found ; i++) {

		lseek(input_img_fd, offsets[i]+8, SEEK_SET);
		if (write(input_img_fd, dataptr, datasize) !=
			sizeof(*dataptr) * datasize) {
			fprintf(stderr, "write() failed: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}

	ret = close(input_img_fd);

	return ret;
}
