// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Bootlin
 *
 * Authors: Boris Brezillon <boris.brezillon@collabora.com>
 *          Miquel Raynal <miquel.raynal@bootlin.com>
 *
 * Overview:
 *   This utility manually flips specified bits in a NAND flash.
 */

#define PROGRAM_NAME "nandflipbits"

#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libmtd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>

#include "common.h"

struct bit_flip {
	uint32_t block;
	uint64_t offset;
	int bit;
	bool done;
};

static void usage(int status)
{
	fprintf(status == EXIT_SUCCESS ? stdout : stderr,
"Usage: "PROGRAM_NAME" [OPTIONS] <device> <bit>@<address> [<bit>@<address>...]\n"
"\n"
"       Test ECC engines, see if they match the specified correction strength:\n"
"         * Reads in raw mode the data from an MTD device\n"
"         * Flips the indicated bit(s)\n"
"         * Write it back in raw mode.\n"
"\n"
"       -h, --help              Display this help and exit\n"
"       -o, --oob               Provided addresses take OOB area into account\n"
"       -q, --quiet             Don't display progress messages\n"
"\n"
	);
	exit(status);
}

static const char *mtd_device;
static bool quiet = false;
static bool oob_mode = false;
static struct bit_flip *bits_to_flip;
static int nbits_to_flip = 0;

static void process_options(int argc, char * const argv[])
{
	int error = 0;
	int i;

	for (;;) {
		int option_index = 0;
		static const char short_options[] = "hoq";
		static const struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"oob", no_argument, 0, 'o'},
			{"quiet", no_argument, 0, 'q'},
			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, short_options,
				    long_options, &option_index);
		if (c == EOF)
			break;

		switch (c) {
		case 'q':
			quiet = true;
			break;
		case 'o':
			oob_mode = true;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case '?':
		default:
			error++;
			break;
		}
	}

	argc -= optind;
	argv += optind;

	/*
	 * There must be at least the MTD device node path argument remaining
	 * and a list of minimum one 'bits-to-flip'.
	 */

	if (argc < 2 || error)
		usage(EXIT_FAILURE);

	/* MTD device */
	mtd_device = argv[0];
	argc--;
	argv++;

	/* Parse the bits to flip */
	nbits_to_flip = argc;
	bits_to_flip = malloc(sizeof(*bits_to_flip) * nbits_to_flip);
	if (!bits_to_flip)
		exit(EXIT_FAILURE);

	for (i = 0; i < nbits_to_flip; i++) {
		struct bit_flip	*bit_to_flip = &bits_to_flip[i];
		char *desc = argv[i];

		bit_to_flip->bit = strtol(desc, &desc, 0);
		if (errno || bit_to_flip->bit > 7)
			goto free_bits;

		if (!desc || *desc++ != '@')
			goto free_bits;

		bit_to_flip->offset = strtol(desc, &desc, 0);
		if (errno)
			goto free_bits;
	}

	return;

free_bits:
	free(bits_to_flip);

	fprintf(stderr, "Invalid bit description\n");

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct mtd_dev_info mtd;
	libmtd_t mtd_desc;
	uint64_t mtdlen;
	uint32_t pagelen, pages_per_blk, blklen;
	uint8_t *buffer;
	int fd, ret, i;

	process_options(argc, argv);

	/* Open the libmtd */
	mtd_desc = libmtd_open();
	if (!mtd_desc) {
		fprintf(stderr, "Cannot initialize libmtd\n");
		ret = EXIT_FAILURE;
		goto free_bits;
	}

	/* Fill in MTD device capability structure */
	ret = mtd_get_dev_info(mtd_desc, mtd_device, &mtd);
	if (ret < 0) {
		fprintf(stderr, "Cannot retrieve MTD device information\n");
		ret = EXIT_FAILURE;
		goto close_lib;
	}

	/* Verify we are using a NAND device */
	if (mtd.type != MTD_NANDFLASH && mtd.type != MTD_MLCNANDFLASH) {
		fprintf(stderr, "%s is not a NAND flash\n", mtd_device);
		ret = EXIT_FAILURE;
		goto close_lib;
	}

	/* Open the MTD device */
	fd = open(mtd_device, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Cannot open %s\n", mtd_device);
		ret = EXIT_FAILURE;
		goto close_lib;
	}

	/* Select raw mode */
	ret = ioctl(fd, MTDFILEMODE, MTD_FILE_MODE_RAW);
	if (ret) {
		fprintf(stderr, "Unavailable raw mode ioctl\n");
		ret = EXIT_FAILURE;
		goto close_fd;
	}

	pagelen = mtd.min_io_size + (oob_mode ? mtd.oob_size : 0);
	pages_per_blk = mtd.eb_size / mtd.min_io_size;
	blklen = pages_per_blk * pagelen;
	mtdlen = (uint64_t)blklen * (uint64_t)mtd.eb_cnt;
	buffer = malloc((mtd.min_io_size + mtd.oob_size) * pages_per_blk);
	if (!buffer) {
		ret = EXIT_FAILURE;
		goto close_fd;
	}

	for (i = 0; i < nbits_to_flip; i++) {
		int page;

		if (bits_to_flip[i].offset >= mtdlen) {
			fprintf(stderr, "Invalid byte offset %lld (max %lld)\n",
				bits_to_flip[i].offset, mtdlen);
			ret = EXIT_FAILURE;
			goto free_buf;
		}

		bits_to_flip[i].block = bits_to_flip[i].offset / blklen;
		bits_to_flip[i].offset %= blklen;
		page = bits_to_flip[i].offset / pagelen;
		bits_to_flip[i].offset = (page *
					  (mtd.min_io_size + mtd.oob_size)) +
					 (bits_to_flip[i].offset % pagelen);
	}

	while (1) {
		struct bit_flip	*bit_to_flip = NULL;
		int blkoffs;
		int bufoffs;

		/* Look for the next bitflip to insert */
		for (i = 0; i < nbits_to_flip; i++) {
			if (bits_to_flip[i].done == false) {
				bit_to_flip = &bits_to_flip[i];
				break;
			}
		}

		if (!bit_to_flip) {
			ret = EXIT_SUCCESS;
			break;
		}

		/* Read the content of all the pages of a block */
		blkoffs = 0;
		bufoffs = 0;
		for (i = 0; i < pages_per_blk; i++) {
			ret = mtd_read(&mtd, fd, bit_to_flip->block, blkoffs,
				       buffer + bufoffs, mtd.min_io_size);
			if (ret) {
				fprintf(stderr, "MTD read failure\n");
				ret = EXIT_FAILURE;
				goto free_buf;
			}

			bufoffs += mtd.min_io_size;

			ret = mtd_read_oob(mtd_desc, &mtd, fd,
					   bit_to_flip->block * mtd.eb_size +
					   blkoffs,
					   mtd.oob_size, buffer + bufoffs);
			if (ret) {
				fprintf(stderr, "MTD OOB read failure\n");
				ret = EXIT_FAILURE;
				goto free_buf;
			}

			bufoffs += mtd.oob_size;
			blkoffs += mtd.min_io_size;
		}

		/* Flip all bits that are located in this particular block */
		for (i = 0; i < nbits_to_flip; i++) {
			unsigned char val, mask;

			if (bits_to_flip[i].block != bit_to_flip->block)
				continue;

			mask = 1 << bits_to_flip[i].bit;
			val = buffer[bits_to_flip[i].offset] & mask;
			if (val)
				buffer[bits_to_flip[i].offset] &= ~mask;
			else
				buffer[bits_to_flip[i].offset] |= mask;
		}

		/* Erase the block */
		ret = mtd_erase(mtd_desc, &mtd, fd, bit_to_flip->block);
		if (ret) {
			fprintf(stderr, "MTD erase failure\n");
			ret = EXIT_FAILURE;
			goto free_buf;
		}

		/* Rewrite the pages, still in raw mode, with the bitflips */
		blkoffs = 0;
		bufoffs = 0;
		for (i = 0; i < pages_per_blk; i++) {
			ret = mtd_write(mtd_desc, &mtd, fd, bit_to_flip->block,
					blkoffs, buffer + bufoffs, mtd.min_io_size,
					buffer + bufoffs + mtd.min_io_size,
					mtd.oob_size,
					MTD_OPS_RAW);
			if (ret) {
				fprintf(stderr, "MTD write failure\n");
				ret = EXIT_FAILURE;
				goto free_buf;
			}

			blkoffs += mtd.min_io_size;
			bufoffs += mtd.min_io_size + mtd.oob_size;
		}

		/* Mark the added bitflips as done */
		for (i = 0; i < nbits_to_flip; i++) {
			if (bits_to_flip[i].block == bit_to_flip->block)
				bits_to_flip[i].done = true;
		}
	}

free_buf:
	free(buffer);
close_fd:
	close(fd);
close_lib:
	libmtd_close(mtd_desc);
free_bits:
	free(bits_to_flip);

	exit(ret);
}
