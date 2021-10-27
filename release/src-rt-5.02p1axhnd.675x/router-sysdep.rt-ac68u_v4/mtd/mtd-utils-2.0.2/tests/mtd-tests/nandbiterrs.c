/*
 * Copyright © 2012 NetCommWireless
 * Iwo Mergler <Iwo.Mergler@netcommwireless.com.au>
 *
 * Copyright © 2015 sigma star gmbh
 * David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 *
 * Test for multi-bit error recovery on a NAND page. This mostly tests the
 * ECC controller / driver.
 *
 * There are two test modes:
 *
 *	0 - artificially inserting bit errors until the ECC fails
 *	    This is the default method and fairly quick. It should
 *	    be independent of the quality of the FLASH.
 *
 *	1 - re-writing the same pattern repeatedly until the ECC fails.
 *	    This method relies on the physics of NAND FLASH to eventually
 *	    generate '0' bits if '1' has been written sufficient times.
 *	    Depending on the NAND, the first bit errors will appear after
 *	    1000 or more writes and then will usually snowball, reaching the
 *	    limits of the ECC quickly.
 *
 *	    The test stops after 10000 cycles, should your FLASH be
 *	    exceptionally good and not generate bit errors before that. Try
 *	    a different page in that case.
 *
 * Please note that neither of these tests will significantly 'use up' any
 * FLASH endurance. Only a maximum of two erase operations will be performed.
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING. If not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#define PROGRAM_NAME "nandbiterrs"

#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libmtd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>

#include "common.h"

/* We don't expect more than this many correctable bit errors per page. */
#define MAXBITS 512

#define KEEP_CONTENTS 0x01
#define MODE_INCREMENTAL 0x02
#define MODE_OVERWRITE 0x04
#define PAGE_ERASED 0x08

static int peb = -1, page = -1, max_overwrite = -1, seed = -1;
static const char *mtddev;

static unsigned char *wbuffer, *rbuffer, *old_data;
static int fd, pagesize, pagecount, flags;
static struct mtd_dev_info mtd;
static libmtd_t mtd_desc;

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "keep", no_argument, NULL, 'k' },
	{ "peb", required_argument, NULL, 'b' },
	{ "page", required_argument, NULL, 'p' },
	{ "seed", required_argument, NULL, 's' },
	{ "erased", no_argument, NULL, 'e' },
	{ "writes", required_argument, NULL, 'w' },
	{ "incremental", no_argument, NULL, 'i' },
	{ "overwrite", no_argument, NULL, 'o' },
	{ NULL, 0, NULL, 0 },
};

static NORETURN void usage(int status)
{
	fputs(
	"Usage: "PROGRAM_NAME" [OPTIONS] <device>\n\n"
	"Common options:\n"
	"  -h, --help          Display this help output\n"
	"  -k, --keep          Restore existing contents after test\n"
	"  -b, --peb <num>     Use this physical erase block\n"
	"  -p, --page <num>    Use this page within the erase block\n"
	"  -s, --seed <num>    Specify seed for PRNG\n"
	"  -e, --erased        Test erased pages instead of written pages\n\n"
	"Options controling test mode:\n"
	"  -i, --incremental   Manually insert bit errors until ECC fails\n"
	"  -o, --overwrite     Rewrite page until bits flip and ECC fails\n\n"
	"Test mode specific options:\n"
	"  -w, --writes <num>  Number of writes (default 10000)\n",
	status==EXIT_SUCCESS ? stdout : stderr);
	exit(status);
}

static long read_num(int opt, const char *arg)
{
	char *end;
	long num;

	num = strtol(arg, &end, 0);

	if (!end || *end != '\0') {
		fprintf(stderr, "-%c: expected integer argument\n", opt);
		exit(EXIT_FAILURE);
	}
	return num;
}

static void process_options(int argc, char **argv)
{
	int c;

	while (1) {
		c = getopt_long(argc, argv, "hkb:p:s:eiow:", options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'k':
			if (flags & KEEP_CONTENTS)
				goto failmulti;
			flags |= KEEP_CONTENTS;
			break;
		case 'b':
			if (peb >= 0)
				goto failmulti;
			peb = read_num(c, optarg);
			if (peb < 0)
				goto failarg;
			break;
		case 'i':
			if (flags & (MODE_INCREMENTAL|MODE_OVERWRITE))
				goto failmultimode;
			flags |= MODE_INCREMENTAL;
			break;
		case 'o':
			if (flags & (MODE_INCREMENTAL|MODE_OVERWRITE))
				goto failmultimode;
			flags |= MODE_OVERWRITE;
			break;
		case 'w':
			if (max_overwrite > 0)
				goto failmulti;
			max_overwrite = read_num(c, optarg);
			if (max_overwrite <= 0)
				goto failarg;
			break;
		case 's':
			if (seed >= 0)
				goto failmulti;
			seed = read_num(c, optarg);
			if (seed < 0)
				goto failarg;
			break;
		case 'p':
			if (page > 0)
				goto failmulti;
			page = read_num(c, optarg);
			if (page < 0)
				goto failarg;
			break;
		case 'e':
			flags |= PAGE_ERASED;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
		default:
			exit(EXIT_FAILURE);
		}
	}

	if (optind < argc)
		mtddev = argv[optind++];
	else
		errmsg_die("No device specified!\n");

	if (optind < argc)
		usage(EXIT_FAILURE);

	if (!(flags & (MODE_OVERWRITE|MODE_INCREMENTAL)))
		errmsg_die("No test mode specified!");

	if ((max_overwrite > 0) && !(flags & MODE_OVERWRITE))
		errmsg_die("Write count specified but mode is not --overwrite!");

	if (max_overwrite < 0)
		max_overwrite = 10000;
	if (peb < 0)
		peb = 0;
	if (page < 0)
		page = 0;
	if (seed < 0)
		seed = 0;
	return;
failmultimode:
	errmsg_die("Test mode specified more than once!");
failmulti:
	errmsg_die("'-%c' specified more than once!", c);
failarg:
	errmsg_die("Invalid argument for '-%c'!", c);
}

/* 'random' bytes from known offsets */
static unsigned char hash(unsigned int offset)
{
	unsigned int v = offset;
	unsigned char c;
	v ^= 0x7f7edfd3;
	v = v ^ (v >> 3);
	v = v ^ (v >> 5);
	v = v ^ (v >> 13);
	c = v & 0xFF;
	/* Reverse bits of result. */
	c = (c & 0x0F) << 4 | (c & 0xF0) >> 4;
	c = (c & 0x33) << 2 | (c & 0xCC) >> 2;
	c = (c & 0x55) << 1 | (c & 0xAA) >> 1;
	return c;
}

static void init_buffer(void)
{
	unsigned int i;

	if (flags & PAGE_ERASED) {
		memset(wbuffer, 0xff, pagesize);
	} else {
		for (i = 0; i < pagesize; ++i)
			wbuffer[i] = hash(i+seed);
	}
}

static int write_page(void)
{
	int raw = flags & PAGE_ERASED;
	int err;

	if (raw && ioctl(fd, MTDFILEMODE, MTD_FILE_MODE_RAW) != 0)
		goto fail_mode;

	err = mtd_write(mtd_desc, &mtd, fd, peb, page*pagesize,
					wbuffer, pagesize, NULL, 0, 0);

	if (err)
		fprintf(stderr, "Failed to write page %d in block %d\n", peb, page);

	if (raw && ioctl(fd, MTDFILEMODE, MTD_FILE_MODE_NORMAL) != 0)
		goto fail_mode;

	return err;
fail_mode:
	perror("MTDFILEMODE");
	return -1;
}

static int rewrite_page(void)
{
	if (ioctl(fd, MTDFILEMODE, MTD_FILE_MODE_RAW) != 0)
		goto fail_mode;

	if (write_page() != 0)
		return -1;

	if (ioctl(fd, MTDFILEMODE, MTD_FILE_MODE_NORMAL) != 0)
		goto fail_mode;

	return 0;
fail_mode:
	perror("MTDFILEMODE");
	return -1;
}

static int read_page(void)
{
	struct mtd_ecc_stats old, new;
	int err = 0;

	if (ioctl(fd, ECCGETSTATS, &old) != 0)
		goto failstats;

	err = mtd_read(&mtd, fd, peb, page*pagesize, rbuffer, pagesize);
	if (err) {
		fputs("Read failed!\n", stderr);
		return -1;
	}

	if (ioctl(fd, ECCGETSTATS, &new) != 0)
		goto failstats;

	if (new.failed > old.failed) {
		fprintf(stderr, "Failed to recover %d bitflips\n",
				new.failed - old.failed);
		return -1;
	}

	return new.corrected - old.corrected;
failstats:
	perror("ECCGETSTATS");
	return -1;
}

static int verify_page(void)
{
	int erased = flags & PAGE_ERASED;
	unsigned int i, errs = 0;

	for (i = 0; i < pagesize; ++i) {
		if (rbuffer[i] != (erased ? 0xff : hash(i+seed)))
			++errs;
	}

	if (errs)
		fputs("ECC failure, invalid data despite read success\n", stderr);

	return errs;
}

/* Finds the first '1' bit in wbuffer and sets it to '0'. */
static int insert_biterror(void)
{
	int bit, mask, byte;

	for (byte = 0; byte < pagesize; ++byte) {
		for (bit = 7, mask = 0x80; bit >= 0; bit--, mask>>=0) {
			if (wbuffer[byte] & mask) {
				wbuffer[byte] &= ~mask;
				printf("Inserted biterror @ %u/%u\n", byte, bit);
				return 0;
			}
		}
	}
	fputs("biterror: Failed to find a '1' bit\n", stderr);
	return -1;
}

/* Writes 'random' data to page and then introduces deliberate bit
 * errors into the page, while verifying each step. */
static int incremental_errors_test(void)
{
	unsigned int errs_per_subpage = 0;
	int count = 0;

	puts("incremental biterrors test");

	init_buffer();

	if (write_page() != 0)
		return -1;

	for (errs_per_subpage = 0; ; ++errs_per_subpage) {
		if (rewrite_page() != 0)
			return -1;

		count = read_page();
		if (count > 0)
			printf("Read reported %d corrected bit errors\n", count);
		if (count < 0) {
			fprintf(stderr, "Read error after %d bit errors per page\n",
				errs_per_subpage);
			return 0;
		}

		if (verify_page() != 0)
			return -1;

		printf("Successfully corrected %d bit errors per subpage\n",
			errs_per_subpage);

		if (insert_biterror() != 0)
			return -1;
	}

	return 0;
}


/* Writes 'random' data to page and then re-writes that same data repeatedly.
   This eventually develops bit errors (bits written as '1' will slowly become
   '0'), which are corrected as far as the ECC is capable of. */
static int overwrite_test(void)
{
	unsigned int i, max_corrected = 0, opno;
	unsigned int bitstats[MAXBITS]; /* bit error histogram. */
	int err = 0;

	memset(bitstats, 0, sizeof(bitstats));

	puts("overwrite biterrors test");

	init_buffer();

	if (write_page() != 0)
		return -1;

	for (opno = 0; opno < max_overwrite; ++opno) {
		err = write_page();
		if (err)
			break;

		err = read_page();
		if (err >= 0) {
			if (err >= MAXBITS) {
				puts("Implausible number of bit errors corrected");
				err = -1;
				break;
			}
			bitstats[err]++;
			if (err > max_corrected) {
				max_corrected = err;
				printf("Read reported %d corrected bit errors\n", err);
			}
		} else {
			err = 0;
			break;
		}

		err = verify_page();
		if (err) {
			bitstats[max_corrected] = opno;
			break;
		}
	}

	/* At this point bitstats[0] contains the number of ops with no bit
	 * errors, bitstats[1] the number of ops with 1 bit error, etc. */
	printf("Bit error histogram (%d operations total):\n", opno);
	for (i = 0; i < max_corrected; ++i) {
		printf("Page reads with %3d corrected bit errors: %d\n",
			i, bitstats[i]);
	}
	return err;
}

int main(int argc, char **argv)
{
	int err = 0, status = EXIT_FAILURE;

	process_options(argc, argv);

	mtd_desc = libmtd_open();
	if (!mtd_desc)
		return errmsg("can't initialize libmtd");

	if (mtd_get_dev_info(mtd_desc, mtddev, &mtd) < 0)
		return errmsg("mtd_get_dev_info failed");

	if (mtd.type!=MTD_MLCNANDFLASH && mtd.type!=MTD_NANDFLASH)
		return errmsg("%s is not a NAND flash!", mtddev);

	pagesize = mtd.subpage_size;
	pagecount = mtd.eb_size / pagesize;

	if (peb >= mtd.eb_cnt)
		return errmsg("Physical erase block %d is out of range!", peb);
	if (page >= pagecount)
		return errmsg("Page number %d is out of range!", page);
	if ((fd = open(mtddev, O_RDWR)) == -1) {
		perror(mtddev);
		return EXIT_FAILURE;
	}

	if (flags & KEEP_CONTENTS) {
		old_data = malloc(mtd.eb_size);
		if (!old_data) {
			perror(NULL);
			goto fail_dev;
		}
		if (mtd_read(&mtd, fd, peb, 0, old_data, mtd.eb_size)) {
			fprintf(stderr, "Reading erase block %d failed!\n", peb);
			goto fail_dev;
		}
	}

	wbuffer = malloc(pagesize);
	if (!wbuffer) {
		perror(NULL);
		goto fail_dev;
	}

	rbuffer = malloc(pagesize);
	if (!rbuffer) {
		perror(NULL);
		goto fail_rbuffer;
	}

	if (mtd_erase(mtd_desc, &mtd, fd, peb)) {
		fprintf(stderr, "Cannot erase block %d\n", peb);
		goto fail_test;
	}

	if (flags & MODE_INCREMENTAL)
		err = incremental_errors_test();
	else if (flags & MODE_OVERWRITE)
		err = overwrite_test();

	status = err ? EXIT_FAILURE : EXIT_SUCCESS;

	if (flags & KEEP_CONTENTS) {
		if (mtd_erase(mtd_desc, &mtd, fd, peb)) {
			fprintf(stderr, "Restoring: Cannot erase block %d\n", peb);
			status = EXIT_FAILURE;
			goto fail_test;
		}

		err = mtd_write(mtd_desc, &mtd, fd, peb, 0,
					old_data, mtd.eb_size, NULL, 0, 0);

		if (err) {
			fputs("Failed restoring old block contents!\n", stderr);
			status = EXIT_FAILURE;
		}
	} else {
		/* We leave the block un-erased in case of test failure. */
		if (err)
			goto fail_test;

		if (mtd_erase(mtd_desc, &mtd, fd, peb)) {
			fprintf(stderr, "Cannot erase block %d\n", peb);
			status = EXIT_FAILURE;
		}
	}
fail_test:
	free(rbuffer);
fail_rbuffer:
	free(wbuffer);
fail_dev:
	close(fd);
	free(old_data);
	return status;
}
