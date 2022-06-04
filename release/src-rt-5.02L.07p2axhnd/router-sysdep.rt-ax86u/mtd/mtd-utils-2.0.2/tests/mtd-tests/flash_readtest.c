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
 * Check MTD device read.
 *
 * Author: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 *
 * Based on linux readtest.c
 * Author: Adrian Hunter <ext-adrian.hunter@nokia.com>
 */
#define PROGRAM_NAME "flash_readtest"

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

#define FLAG_VERBOSE 1

static int peb=-1, skip=-1, count=-1, flags=0, pgcnt, pgsize, fd;
static unsigned char *iobuf, *iobuf1;
static struct mtd_dev_info mtd;
static const char *mtddev;
static libmtd_t mtd_desc;

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "verbose", no_argument, NULL, 'v' },
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
	"  -c, --count <num>   Number of erase blocks to process (default: all)\n"
	"  -s, --skip <num>    Number of blocks to skip\n"
	"  -v, --verbose       Generate more verbose output\n\n",
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
		c = getopt_long(argc, argv, "hb:c:s:v", options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'v':
			if (flags & FLAG_VERBOSE)
				goto failmulti;
			flags |= FLAG_VERBOSE;
			break;
		case 'b':
			if (peb >= 0)
				goto failmulti;
			peb = read_num(c, optarg);
			if (peb < 0)
				goto failarg;
			break;
		case 'c':
			if (count >= 0)
				goto failmulti;
			count = read_num(c, optarg);
			if (count < 0)
				goto failarg;
			break;
		case 's':
			if (skip >= 0)
				goto failmulti;
			skip = read_num(c, optarg);
			if (skip < 0)
				goto failarg;
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
	if (peb < 0)
		peb = 0;
	if (skip < 0)
		skip = 0;
	return;
failmulti:
	errmsg_die("'-%c' specified more than once!", c);
failarg:
	errmsg_die("Invalid argument for '-%c'!", c);
}

static int read_eraseblock_by_page(int ebnum)
{
	unsigned char *buf = iobuf, *oobbuf = iobuf1;
	uint64_t addr = ((uint64_t)ebnum) * ((uint64_t)mtd.eb_size);
	int i, ret;

	for (i = 0; i < pgcnt; ++i) {
		memset(buf, 0, pgsize);
		ret = mtd_read(&mtd, fd, ebnum, i*pgsize, buf, pgsize);
		if (ret) {
			fprintf(stderr, "Error reading block %d, page %d\n", ebnum, i);
			return -1;
		}
		if (mtd.oob_size) {
			ret = mtd_read_oob(mtd_desc, &mtd, fd,
							addr, mtd.oob_size, oobbuf);

			if (ret) {
				fprintf(stderr, "Error reading OOB in block %d, page %d\n",
						ebnum, i);
				return -1;
			}
			oobbuf += mtd.oob_size;
		}
		buf += pgsize;
		addr += pgsize;
	}

	if (flags & FLAG_VERBOSE)
		printf("Successfully read erase block %d\n", ebnum);

	return 0;
}

static void dump_eraseblock(int ebnum)
{
	char line[128];
	int i, j, n;
	int pg, oob;

	printf("dumping eraseblock %d\n", ebnum);
	n = mtd.eb_size;
	for (i = 0; i < n;) {
		char *p = line;

		p += sprintf(p, "%05x: ", i);
		for (j = 0; j < 32 && i < n; j++, i++)
			p += sprintf(p, "%02x", (unsigned int)iobuf[i]);
		printf("%s\n", line);
	}
	if (!mtd.oob_size)
		return;
	printf("dumping oob from eraseblock %d\n", ebnum);
	n = mtd.oob_size;
	for (pg = 0, i = 0; pg < pgcnt; ++pg) {
		for (oob = 0; oob < n;) {
			char *p = line;

			p += sprintf(p, "%05x: ", i);
			for (j = 0; j < 32 && oob < n; ++j, ++oob, ++i)
				p += sprintf(p, "%02x", (unsigned int)iobuf1[i]);
			printf("%s\n", line);
		}
	}
	putchar('\n');
}

int main(int argc, char **argv)
{
	int status = EXIT_SUCCESS, i, ret, blk;

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
		count = mtd.eb_cnt;

	if (peb >= mtd.eb_cnt)
		return errmsg("Physical erase block %d is out of range!\n", peb);

	if ((peb + (count - 1)*(skip + 1)) >= mtd.eb_cnt) {
		return errmsg("Given block range exceeds block count of %d!\n",
					mtd.eb_cnt);
	}

	iobuf = xmalloc(mtd.eb_size);
	iobuf1 = xmalloc(mtd.eb_size);

	if ((fd = open(mtddev, O_RDWR)) == -1) {
		perror(mtddev);
		status = EXIT_FAILURE;
		goto out;
	}

	/* Read all eraseblocks 1 page at a time */
	puts("testing page read");

	for (i = 0; i < count; ++i) {
		blk = peb + i*(skip+1);

		if (mtd_is_bad(&mtd, fd, blk)) {
			printf("Skipping bad block %d\n", blk);
			continue;
		}
		ret = read_eraseblock_by_page(blk);
		if (ret && (flags & FLAG_VERBOSE)) {
			dump_eraseblock(blk);
			status = EXIT_FAILURE;
		}
	}
out:
	free(iobuf);
	free(iobuf1);
	return status;
}
