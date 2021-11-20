/*
 * Copyright (C) 2006-2008 Nokia Corporation
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
 * Test random reads, writes and erases on MTD device.
 *
 * Author: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 *
 * Based on linux stresstest.c
 * Author: Adrian Hunter <ext-adrian.hunter@nokia.com>
 */
#define PROGRAM_NAME "flash_stress"

#define KEEP_CONTENTS 0x01
#define COUNT_CHANGED 0x02
#define SEED_SET 0x04

#include <mtd/mtd-user.h>
#include <unistd.h>
#include <stdlib.h>
#include <libmtd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#include "common.h"

static struct mtd_dev_info mtd;
static const char *mtddev;
static libmtd_t mtd_desc;
static int fd;

static unsigned char *writebuf;
static unsigned char *readbuf;
static unsigned char *old;
static unsigned char *bbt;

static int pgsize;
static int pgcnt;

static int count = 10000;
static int flags = 0;

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "keep", no_argument, NULL, 'k' },
	{ "seed", required_argument, NULL, 's' },
	{ "count", required_argument, NULL, 'c' },
	{ NULL, 0, NULL, 0 },
};

static NORETURN void usage(int status)
{
	fputs(
	"Usage: "PROGRAM_NAME" [OPTIONS] <device>\n\n"
	"Options:\n"
	"  -h, --help         Display this help output\n"
	"  -c, --count <num>  Number of operations to do (default is 10000)\n"
	"  -s, --seed <num>   Seed for pseudor random number generator\n"
	"  -k, --keep         Restore existing contents after test\n",
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
		c = getopt_long(argc, argv, "hc:s:k", options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'k':
			if (flags & KEEP_CONTENTS)
				goto failmulti;
			flags |= KEEP_CONTENTS;
			break;
		case 's':
			if (flags & SEED_SET)
				goto failmulti;
			srand(read_num(c, optarg));
			flags |= SEED_SET;
			break;
		case 'c':
			if (flags & COUNT_CHANGED)
				goto failmulti;
			count = read_num(c, optarg);
			if (count <= 0)
				goto failarg;
			flags |= COUNT_CHANGED;
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
	if (!(flags & SEED_SET))
		srand(time(NULL));
	return;
failmulti:
	errmsg_die("'-%c' specified more than once!\n", c);
failarg:
	errmsg_die("Invalid argument for '-%c'!\n", c);
}

static int rand_eb(void)
{
	unsigned int eb;

	/* Read or write up 2 eraseblocks at a time - hence 'mtd.eb_cnt - 1' */
	do {
		eb = rand() % (mtd.eb_cnt - 1);
	} while (bbt[eb]);

	return eb;
}

static int do_read(void)
{
	int eb = rand_eb();
	int offs = rand() % pgcnt;
	int len = rand() % (pgcnt - offs);

	offs *= pgsize;
	len *= pgsize;
	return mtd_read(&mtd, fd, eb, offs, readbuf, len);
}

static int do_write(void)
{
	int eb = rand_eb(), err, err1;
	int offs = rand() % pgcnt;
	int len = rand() % (pgcnt - offs);

	offs *= pgsize;
	len *= pgsize;

	if (flags & KEEP_CONTENTS) {
		err = mtd_read(&mtd, fd, eb, 0, old, mtd.eb_size);
		if (err) {
			fputs("Error backing up old erase block contents\n", stderr);
			return -1;
		}
	}

	err = mtd_erase(mtd_desc, &mtd, fd, eb);
	if (err)
		goto out;

	err = mtd_write(mtd_desc, &mtd, fd, eb, offs,
			writebuf, len, NULL, 0, 0);
	if (err)
		goto out;

	err = 0;
out:
	if (flags & KEEP_CONTENTS) {
		if (mtd_erase(mtd_desc, &mtd, fd, eb)) {
			fprintf(stderr, "mtd_erase: PEB %d", eb);
			return -1;
		}

		err1 = mtd_write(mtd_desc, &mtd, fd, eb, 0,
					old, mtd.eb_size, NULL, 0, 0);

		if (err1) {
			fprintf(stderr, "Failed to restore old contents\n");
			return -1;
		}
	}
	return err;
}

static void scan_for_bad_eraseblocks(unsigned int eb, int ebcnt)
{
	int i, bad = 0;

	puts("scanning for bad eraseblocks");

	for (i = 0; i < ebcnt; ++i) {
		bbt[i] = mtd_is_bad(&mtd, fd, eb + i) ? 1 : 0;
		if (bbt[i])
			bad += 1;
	}

	printf("scanned %d eraseblocks, %d are bad\n", ebcnt, bad);
}

int main(int argc, char **argv)
{
	int status = EXIT_FAILURE, i, op, err;

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

	readbuf = xmalloc(mtd.eb_size);
	writebuf = xmalloc(mtd.eb_size);
	bbt = xzalloc(mtd.eb_cnt);

	if (flags & KEEP_CONTENTS)
		old = xmalloc(mtd.eb_size);

	for (i = 0; i < mtd.eb_size; ++i)
		writebuf[i] = rand();

	/* Open device file */
	if ((fd = open(mtddev, O_RDWR)) == -1) {
		perror(mtddev);
		goto out;
	}

	/* Do operations */
	scan_for_bad_eraseblocks(0, mtd.eb_cnt);

	puts("doing operations");
	for (op = 0; op < count; op++) {
		if ((op & 1023) == 0)
			printf("%d operations done\n", op);
		err = (rand() & 1) ? do_read() : do_write();
		if (err)
			goto out;
	}
	printf("finished, %d operations done\n", op);

	status = EXIT_SUCCESS;
out:
	close(fd);
	free(bbt);
	free(writebuf);
	free(readbuf);
	free(old);
	return status;
}
