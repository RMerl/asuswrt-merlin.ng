/*
 * Copyright (C) 2006-2007 Nokia Corporation
 * Copyright (C) 2016 sigma star gmbh
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
 * Test sub-page read and write on MTD device.
 *
 * Author: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 *
 * Based on linux subpagetest.c
 * Author: Adrian Hunter <ext-adrian.hunter@nokia.com>
 */
#define PROGRAM_NAME "nandsubpagetest"

#include <mtd/mtd-user.h>
#include <unistd.h>
#include <stdlib.h>
#include <libmtd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#include "common.h"

#define KEEP_CONTENTS 0x01
#define SEED_SET 0x02

static struct mtd_dev_info mtd;
static const char *mtddev;
static libmtd_t mtd_desc;

static unsigned char *bbt=NULL, *writebuf=NULL, *backup=NULL, *readbuf=NULL;
static int peb = -1, seed = -1, skip = -1, ebcnt = -1, flags = 0;
static int fd, bufsize;
static unsigned int rnd_state;

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "keep", no_argument, NULL, 'k' },
	{ "peb", required_argument, NULL, 'b' },
	{ "count", required_argument, NULL, 'c' },
	{ "skip", required_argument, NULL, 's' },
	{ "seed", required_argument, NULL, 'S' },
	{ NULL, 0, NULL, 0 },
};

static NORETURN void usage(int status)
{
	fputs(
	"Usage: "PROGRAM_NAME" [OPTIONS] <device>\n\n"
	"Options:\n"
	"  -h, --help         Display this help output\n"
	"  -b, --peb <num>    Index of the first erase block to use\n"
	"  -c, --count <num>  Number of erase blocks to use (default all)\n"
	"  -s, --skip <num>   Number of erase blocks to skip\n"
	"  -S, --seed <num>   Seed for pseudor random number generator\n"
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
		c = getopt_long(argc, argv, "hb:c:s:Sk", options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'b':
			if (peb >= 0)
				goto failmulti;
			peb = read_num(c, optarg);
			if (peb < 0)
				goto failarg;
			break;
		case 'c':
			if (ebcnt >= 0)
				goto failmulti;
			ebcnt = read_num(c, optarg);
			if (ebcnt < 0)
				goto failarg;
			break;
		case 's':
			if (skip >= 0)
				goto failmulti;
			skip = read_num(c, optarg);
			if (skip < 0)
				goto failarg;
			break;
		case 'S':
			if (flags & SEED_SET)
				goto failmulti;
			seed = read_num(c, optarg);
			flags |= SEED_SET;
			break;
		case 'k':
			if (flags & KEEP_CONTENTS)
				goto failmulti;
			flags |= KEEP_CONTENTS;
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
	if (!(flags & SEED_SET))
		seed = time(NULL);
	if (skip < 0)
		skip = 0;
	return;
failmulti:
	errmsg_die("'-%c' specified more than once!", c);
failarg:
	errmsg_die("Invalid argument for '-%c'!", c);
}

static int write_eraseblock(int ebnum)
{
	int i, j, err, off = 0;

	printf("writing first 2 sub-pages on PEB %d\n", ebnum);
	for (j = 0; j < 2; ++j) {
		for (i = 0; i < mtd.subpage_size; ++i)
			writebuf[i] = rand_r(&rnd_state);

		err = mtd_write(mtd_desc, &mtd, fd, ebnum, off, writebuf,
						mtd.subpage_size, NULL, 0, 0);
		if (err)
			return -1;

		off += mtd.subpage_size;
	}
	return 0;
}

static int write_eraseblock2(int ebnum)
{
	int err, off = 0, i, k;

	printf("writing with exponential offsets & sizes on PEB %d\n", ebnum);
	for (k = 1; k < 33; ++k) {
		if (off + (mtd.subpage_size * k) > mtd.eb_size)
			break;

		for (i = 0; i < (mtd.subpage_size * k); ++i)
			writebuf[i] = rand_r(&rnd_state);

		err = mtd_write(mtd_desc, &mtd, fd, ebnum, off,
						writebuf, mtd.subpage_size * k, NULL, 0, 0);
		if (err)
			return -1;

		off += mtd.subpage_size * k;
	}
	return 0;
}

static void print_subpage(unsigned char *p)
{
	int i, j;

	for (i = 0; i < mtd.subpage_size; ) {
		for (j = 0; i < mtd.subpage_size && j < 32; ++i, ++j)
			fprintf(stderr, "%02x", *p++);
		fprintf(stderr, "\n");
	}
}

static int verify_eraseblock(int ebnum)
{
	int i, j, ret = 0, off = 0;

	printf("verifying first 2 sub-pages of PEB %d\n", ebnum);
	for (j = 0; j < 2; ++j) {
		for (i = 0; i < mtd.subpage_size; ++i)
			writebuf[i] = rand_r(&rnd_state);

		memset(readbuf, 0, mtd.subpage_size);
		if (mtd_read(&mtd, fd, ebnum, off, readbuf, mtd.subpage_size))
			return -1;

		if (memcmp(readbuf, writebuf, mtd.subpage_size)) {
			fprintf(stderr, "error: verify failed at PEB %d, offset %#x\n",
					ebnum, off);
			fputs("------------- written----------------\n", stderr);
			print_subpage(writebuf);
			fputs("------------- read ------------------\n", stderr);
			print_subpage(readbuf);
			fputs("-------------------------------------\n", stderr);
			ret = -1;
		}
		off += mtd.subpage_size;
	}
	return ret;
}

static int verify_eraseblock2(int ebnum)
{
	int ret = 0, i, k, off = 0;

	printf("verifying exponential offset & size writes on PEB %d\n", ebnum);
	for (k = 1; k < 33; ++k) {
		if (off + (mtd.subpage_size * k) > mtd.eb_size)
			break;

		for (i = 0; i < (mtd.subpage_size * k); ++i)
			writebuf[i] = rand_r(&rnd_state);

		memset(readbuf, 0, mtd.subpage_size * k);
		if (mtd_read(&mtd, fd, ebnum, off, readbuf, mtd.subpage_size * k))
			return -1;

		if (memcmp(readbuf, writebuf, mtd.subpage_size * k)) {
			fprintf(stderr, "error: verify failed at PEB %d, offset %#x\n",
					ebnum, off);
			ret = -1;
		}
		off += mtd.subpage_size * k;
	}
	return ret;
}

static int verify_eraseblock_ff(int ebnum)
{
	int j, ret = 0, off = 0;

	memset(writebuf, 0xFF, mtd.subpage_size);

	for (j = 0; j < mtd.eb_size / mtd.subpage_size; ++j) {
		memset(readbuf, 0, mtd.subpage_size);
		if (mtd_read(&mtd, fd, ebnum, off, readbuf, mtd.subpage_size))
			return -1;

		if (memcmp(readbuf, writebuf, mtd.subpage_size)) {
			fprintf(stderr, "error: verify 0xff failed at PEB %d, "
					"offset %#x\n", ebnum, off);
			ret = -1;
		}
		off += mtd.subpage_size;
	}
	return ret;
}

static int verify_all_eraseblocks_ff(void)
{
	int i, eb, err;

	puts("verifying all eraseblocks for 0xff");
	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;

		eb = peb + i * (skip + 1);
		err = verify_eraseblock_ff(eb);
		if (err)
			return err;
	}
	printf("verified %d eraseblocks\n", ebcnt);
	return 0;
}

static int erase_good_eraseblocks(void)
{
	int i, eb;

	printf("erasing good eraseblocks\n");
	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;
		eb = peb + i * (skip + 1);
		if (mtd_erase(mtd_desc, &mtd, fd, eb))
			return -1;
	}
	return 0;
}

static int remove_test_data(void)
{
	if (erase_good_eraseblocks())
		return -1;
	if (verify_all_eraseblocks_ff())
		return -1;
	return 0;
}

int main(int argc, char **argv)
{
	int i, eb, err = 0, status = EXIT_FAILURE;
	unsigned char *backupptr;

	process_options(argc, argv);

	mtd_desc = libmtd_open();
	if (!mtd_desc)
		return errmsg("can't initialize libmtd");
	if (mtd_get_dev_info(mtd_desc, mtddev, &mtd) < 0)
		return errmsg("mtd_get_dev_info failed");
	if (mtd.type!=MTD_MLCNANDFLASH && mtd.type!=MTD_NANDFLASH)
		return errmsg("%s is not a NAND flash!", mtddev);
	if (ebcnt < 0)
		ebcnt = (mtd.eb_cnt - peb) / (skip + 1);
	if (peb >= mtd.eb_cnt)
		return errmsg("physical erase block %d is out of range!", peb);
	eb = peb + (ebcnt - 1)*(skip + 1);
	if (eb >= mtd.eb_cnt)
		return errmsg("last physical erase block %d is out of range!", eb);

	bufsize = mtd.subpage_size * 32;
	writebuf = xmalloc(bufsize);
	readbuf = xmalloc(bufsize);
	bbt = xzalloc(ebcnt);

	if ((fd = open(mtddev, O_RDWR)) == -1) {
		perror(mtddev);
		goto out_cleanup;
	}

	/* find bad blocks */
	for (i = 0; i < ebcnt; ++i) {
		eb = peb + i * (skip + 1);
		bbt[i] = mtd_is_bad(&mtd, fd, eb);

		if (bbt[i])
			printf("ignoring bad erase block %d\n", eb);
	}

	/* create block backup */
	if (flags & KEEP_CONTENTS) {
		eb = 0;
		for (i = 0; i < ebcnt; ++i) {
			if (!bbt[i])
				++eb;
		}
		backup = malloc(mtd.eb_size * eb);
		if (!backup) {
			fprintf(stderr, "not enough memory to keep block contents!\n");
			goto out_cleanup;
		}
		printf("reading %d blocks into memory\n", eb);
		backupptr = backup;
		for (i = 0; i < ebcnt; ++i) {
			if (bbt[i])
				continue;
			eb = peb + i*(skip+1);
			err = mtd_read(&mtd, fd, eb, 0, backupptr, mtd.eb_size);
			if (err) {
				fprintf(stderr, "error reading block %d!\n", eb);
				goto out_cleanup;
			}
			backupptr += mtd.eb_size;
		}
	}

	/* write first 2 sub-pages of each block */
	if (remove_test_data())
		goto out;

	rnd_state = seed;
	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;
		err = write_eraseblock(i);
		if (err)
			goto out;
	}

	rnd_state = seed;
	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;
		err = verify_eraseblock(i);
		if (err)
			goto out;
	}

	/* write with exponential offset & size */
	if (remove_test_data())
		goto out;

	rnd_state = seed;
	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;
		err = write_eraseblock2(i);
		if (err)
			goto out;
	}

	rnd_state = seed;
	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;
		err = verify_eraseblock2(i);
		if (err)
			goto out;
	}

	if (remove_test_data())
		goto out;

	status = EXIT_SUCCESS;
out:
	if (flags & KEEP_CONTENTS) {
		puts("restoring original contents");
		backupptr = backup;
		for (i = 0; i < ebcnt; ++i) {
			if (bbt[i])
				continue;
			eb = peb + i*(skip+1);
			if (status != EXIT_SUCCESS && mtd_erase(mtd_desc, &mtd, fd, eb))
				fprintf(stderr, "error erasing block %d!\n", eb);

			err = mtd_write(mtd_desc, &mtd, fd, eb, 0,
							backupptr, mtd.eb_size, NULL, 0, 0);
			if (err) {
				fprintf(stderr, "error restoring block %d!\n", eb);
				status = EXIT_FAILURE;
			}
			backupptr += mtd.eb_size;
		}
	}
out_cleanup:
	free(bbt);
	free(readbuf);
	free(writebuf);
	free(backup);
	close(fd);
	return status;
}
