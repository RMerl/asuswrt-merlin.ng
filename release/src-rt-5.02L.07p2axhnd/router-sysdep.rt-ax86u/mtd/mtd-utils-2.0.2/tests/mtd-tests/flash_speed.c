/*
 * Copyright (C) 2007 Nokia Corporation
 * Copyright (C) 2015 sigma star gmbh
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
 *
 * Test read and write speed of a MTD device.
 *
 * Author: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 *
 * Based on linux flash_speed.c
 * Author: Adrian Hunter <adrian.hunter@nokia.com>
 */
#define DESTRUCTIVE 0x01

#define PROGRAM_NAME "flash_speed"

#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libmtd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#include "common.h"

static struct mtd_dev_info mtd;
static unsigned char *iobuf;
static unsigned char *bbt;
static const char *mtddev;
static libmtd_t mtd_desc;
static int fd;

static int peb=-1, count=-1, skip=-1, flags=0;
static struct timespec start, finish;
static int pgsize, pgcnt;
static int goodebcnt;

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "destructive", no_argument, NULL, 'd' },
	{ "peb", required_argument, NULL, 'b' },
	{ "count", required_argument, NULL, 'c' },
	{ "skip", required_argument, NULL, 's' },
	{ NULL, 0, NULL, 0 },
};

static NORETURN void usage(int status)
{
	fputs(
	"Usage: "PROGRAM_NAME" [OPTIONS] <device>\n\n"
	"Common options:\n"
	"  -h, --help          Display this help output\n"
	"  -b, --peb <num>     Start from this physical erase block\n"
	"  -c, --count <num>   Number of erase blocks to use (default: all)\n"
	"  -s, --skip <num>    Number of blocks to skip\n"
	"  -d, --destructive   Run destructive (erase and write speed) tests\n",
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
		c = getopt_long(argc, argv, "hb:c:s:d", options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage(EXIT_SUCCESS);
		case 'b':
			if (peb >= 0)
				goto failmulti;
			peb = read_num(c, optarg);
			if (peb < 0)
				goto failarg;
			break;
		case 'c':
			if (count > 0)
				goto failmulti;
			count = read_num(c, optarg);
			if (count <= 0)
				goto failarg;
			break;
		case 's':
			if (skip >= 0)
				goto failmulti;
			skip = read_num(c, optarg);
			if (skip < 0)
				goto failarg;
			break;
		case 'd':
			if (flags & DESTRUCTIVE)
				goto failmulti;
			flags |= DESTRUCTIVE;
			break;
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
	if (peb < 0)
		peb = 0;
	if (skip < 0)
		skip = 0;
	if (count < 0)
		count = 1;
	return;
failmulti:
	errmsg_die("'-%c' specified more than once!\n", c);
failarg:
	errmsg_die("Invalid argument for '-%c'!\n", c);
}

static int write_eraseblock(int ebnum)
{
	int err = mtd_write(mtd_desc, &mtd, fd, ebnum, 0,
				iobuf, mtd.eb_size, NULL, 0, 0);
	if (err)
		fprintf(stderr, "Error writing block %d!\n", ebnum);
	return err;
}

static int read_eraseblock(int ebnum)
{
	int err = mtd_read(&mtd, fd, ebnum, 0, iobuf, mtd.eb_size);
	if (err)
		fprintf(stderr, "Error writing block %d!\n", ebnum);
	return err;
}

static int write_eraseblock_by_page(int ebnum)
{
	void *buf = iobuf;
	int i, err = 0;

	for (i = 0; i < pgcnt; ++i) {
		err = mtd_write(mtd_desc, &mtd, fd, ebnum, i * pgsize,
						buf, pgsize, NULL, 0, 0);
		if (err) {
			fprintf(stderr, "Error writing block %d, page %d!\n",
					ebnum, i);
			break;
		}
		buf += pgsize;
	}

	return err;
}

static int write_eraseblock_by_2pages(int ebnum)
{
	int i, n = pgcnt / 2, err = 0;
	size_t sz = pgsize * 2;
	void *buf = iobuf;

	for (i = 0; i < n; ++i) {
		err = mtd_write(mtd_desc, &mtd, fd, ebnum, i * sz,
						buf, sz, NULL, 0, 0);
		if (err) {
			fprintf(stderr, "Error writing block %d, page %d + %d!\n",
					ebnum, i*2, i*2+1);
			return err;
		}
		buf += sz;
	}
	if (pgcnt % 2) {
		err = mtd_write(mtd_desc, &mtd, fd, ebnum, i * sz,
						buf, pgsize, NULL, 0, 0);
		if (err) {
			fprintf(stderr, "Error reading block %d, page %d!\n",
					ebnum, i*2);
		}
	}
	return err;
}

static int read_eraseblock_by_page(int ebnum)
{
	void *buf = iobuf;
	int i, err = 0;

	for (i = 0; i < pgcnt; ++i) {
		err = mtd_read(&mtd, fd, ebnum, i * pgsize, iobuf, pgsize);
		if (err) {
			fprintf(stderr, "Error reading block %d, page %d!\n",
					ebnum, i);
			break;
		}
		buf += pgsize;
	}

	return err;
}

static int read_eraseblock_by_2pages(int ebnum)
{
	int i, n = pgcnt / 2, err = 0;
	size_t sz = pgsize * 2;
	void *buf = iobuf;

	for (i = 0; i < n; ++i) {
		err = mtd_read(&mtd, fd, ebnum, i * sz, iobuf, sz);
		if (err) {
			fprintf(stderr, "Error reading block %d, page %d + %d!\n",
					ebnum, i*2, i*2+1);
			return err;
		}
		buf += sz;
	}
	if (pgcnt % 2) {
		err = mtd_read(&mtd, fd, ebnum, i * sz, iobuf, pgsize);
		if (err) {
			fprintf(stderr, "Error reading block %d, page %d!\n",
					ebnum, i*2);
		}
	}

	return err;
}

static void start_timing(void)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
}

static void stop_timing(void)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
}

static long calc_speed(void)
{
	long ms;

	ms = (finish.tv_sec - start.tv_sec) * 1000L;
	ms += (finish.tv_nsec - start.tv_nsec) / 1000000L;

	if (ms <= 0)
		return 0;

	return ((long)goodebcnt * (mtd.eb_size / 1024L) * 1000L) / ms;
}

static void scan_for_bad_eraseblocks(unsigned int eb, int ebcnt, int ebskip)
{
	int i, bad = 0;

	puts("scanning for bad eraseblocks");

	for (i = 0; i < ebcnt; ++i) {
		bbt[i] = mtd_is_bad(&mtd, fd, eb + i*(ebskip+1)) ? 1 : 0;
		if (bbt[i])
			bad += 1;
	}

	printf("scanned %d eraseblocks, %d are bad\n", ebcnt, bad);
}

static int erase_good_eraseblocks(unsigned int eb, int ebcnt, int ebskip)
{
	int err = 0, block;
	unsigned int i;

	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;
		block = eb + i*(ebskip+1);
		err = mtd_erase(mtd_desc, &mtd, fd, block);
		if (err)
			fprintf(stderr, "Error erasing block %d!\n", block);
	}

	return err;
}

#define TIME_OP_PER_PEB( op )\
		start_timing();\
		for (i = 0; i < count; ++i) {\
			if (bbt[i])\
				continue;\
			err = op(peb + i*(skip+1));\
			if (err)\
				goto out;\
		}\
		stop_timing();\
		speed = calc_speed()

int main(int argc, char **argv)
{
	int err, i, blocks, j, k, status = EXIT_FAILURE;
	long speed;

	process_options(argc, argv);

	mtd_desc = libmtd_open();
	if (!mtd_desc)
		return errmsg("can't initialize libmtd");

	if (mtd_get_dev_info(mtd_desc, mtddev, &mtd) < 0)
		return errmsg("mtd_get_dev_info failed");

	if (mtd.subpage_size == 1) {
		puts("not NAND flash, assume page size is 512 bytes.");
		pgsize = 512;
	} else {
		pgsize = mtd.subpage_size;
	}

	pgcnt = mtd.eb_size / pgsize;

	if (count < 0)
		count = mtd.eb_size;

	if (peb >= mtd.eb_cnt)
		return errmsg("Physical erase block %d is out of range!\n", peb);

	if ((peb + (count - 1)*(skip + 1)) >= mtd.eb_cnt) {
		return errmsg("Given block range exceeds block count of %d!\n",
					mtd.eb_cnt);
	}

	iobuf = xmalloc(mtd.eb_size);
	bbt = xzalloc(count);

	if ((fd = open(mtddev, O_RDWR)) == -1) {
		perror(mtddev);
		goto outfree;
	}

	for (i = 0; i < mtd.eb_size; ++i)
		iobuf[i] = rand();

	scan_for_bad_eraseblocks(peb, count, skip);

	for (i = 0; i < count; ++i) {
		if (!bbt[i])
			goodebcnt++;
	}

	/* Write all eraseblocks, 1 eraseblock at a time */
	if (flags & DESTRUCTIVE) {
		err = erase_good_eraseblocks(peb, count, skip);
		if (err)
			goto out;

		puts("testing eraseblock write speed");
		TIME_OP_PER_PEB(write_eraseblock);
		printf("eraseblock write speed is %ld KiB/s\n", speed);
	}

	/* Read all eraseblocks, 1 eraseblock at a time */
	puts("testing eraseblock read speed");
	TIME_OP_PER_PEB(read_eraseblock);
	printf("eraseblock read speed is %ld KiB/s\n", speed);

	/* Write all eraseblocks, 1 page at a time */
	if (flags & DESTRUCTIVE) {
		err = erase_good_eraseblocks(peb, count, skip);
		if (err)
			goto out;

		puts("testing page write speed");
		TIME_OP_PER_PEB(write_eraseblock_by_page);
		printf("page write speed is %ld KiB/s\n", speed);
	}

	/* Read all eraseblocks, 1 page at a time */
	puts("testing page read speed");
	TIME_OP_PER_PEB(read_eraseblock_by_page);
	printf("page read speed is %ld KiB/s\n", speed);

	/* Write all eraseblocks, 2 pages at a time */
	if (flags & DESTRUCTIVE) {
		err = erase_good_eraseblocks(peb, count, skip);
		if (err)
			goto out;

		puts("testing 2 page write speed");
		TIME_OP_PER_PEB(write_eraseblock_by_2pages);
		printf("2 page write speed is %ld KiB/s\n", speed);
	}

	/* Read all eraseblocks, 2 pages at a time */
	puts("testing 2 page read speed");
	TIME_OP_PER_PEB(read_eraseblock_by_2pages);
	printf("2 page read speed is %ld KiB/s\n", speed);

	/* Erase all eraseblocks */
	if (flags & DESTRUCTIVE) {
		puts("Testing erase speed");
		start_timing();
		err = erase_good_eraseblocks(peb, count, skip);
		if (err)
			goto out;
		stop_timing();
		speed = calc_speed();
		printf("erase speed is %ld KiB/s\n", speed);
	}

	/* Multi-block erase all eraseblocks */
	if (!skip) {
		for (k = 1; k < 7; ++k) {
			blocks = 1 << k;
			printf("Testing %dx multi-block erase speed\n", blocks);
			start_timing();
			for (i = 0; i < count; ) {
				for (j = 0; j < blocks && (i + j) < count; ++j)
					if (bbt[i + j])
						break;
				if (j < 1) {
					++i;
					continue;
				}
				err = mtd_erase_multi(mtd_desc, &mtd, fd, i, j);
				if (err)
					goto out;
				i += j;
			}
			stop_timing();
			speed = calc_speed();
			printf("%dx multi-block erase speed is %ld KiB/s\n",
					blocks, speed);
		}
	}

	puts("finished");
	status = EXIT_SUCCESS;
out:
	close(fd);
outfree:
	free(iobuf);
	free(bbt);
	return status;
}
