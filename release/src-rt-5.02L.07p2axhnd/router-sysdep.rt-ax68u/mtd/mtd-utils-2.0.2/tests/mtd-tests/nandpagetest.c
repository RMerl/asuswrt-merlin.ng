/*
 * Copyright (C) 2006-2008 Nokia Corporation
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
 * Test page read and write on MTD device.
 *
 * Author: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 *
 * Based on linux pagetest.c
 * Author: Adrian Hunter <ext-adrian.hunter@nokia.com>
 */
#define PROGRAM_NAME "nandpagetest"

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

static unsigned char *bbt=NULL, *writebuf=NULL, *backup=NULL;
static unsigned char *twopages=NULL, *boundary=NULL;
static int peb = -1, seed = -1, skip = -1, ebcnt = -1, flags = 0;
static int fd, bufsize, pgsize, pgcnt;
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
	"  -c, --count <num>  Number of erase blocks to use (at least 2, default all)\n"
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
	if (ebcnt >= 0 && ebcnt < 2)
		errmsg_die("Cannot run with less than two blocks.");
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
	int i;

	for (i = 0; i < mtd.eb_size; ++i)
		writebuf[i] = rand_r(&rnd_state);

	return mtd_write(mtd_desc, &mtd, fd, ebnum, 0,
				writebuf, mtd.eb_size, NULL, 0, 0);
}

static void get_first_and_last_block(int *first, int *last)
{
	int i;

	*first = peb;
	for (i = 0; i < ebcnt && bbt[i]; ++i)
		*first += skip + 1;

	*last = peb + (ebcnt - 1) * (skip + 1);
	for (i = 0; i < ebcnt && bbt[ebcnt - i - 1]; ++i)
		*last -= skip + 1;
}

/* Do a read to set the internal dataRAMs to different data */
static int flush_data_rams(int eb0, int ebn)
{
	int err;
	err = mtd_read(&mtd, fd, eb0, 0, twopages, bufsize);
	if (err)
		return err;
	err = mtd_read(&mtd, fd, ebn, mtd.eb_size-bufsize,
					twopages, bufsize);
	if (err)
		return err;
	memset(twopages, 0, bufsize);
	return 0;
}

static int verify_eraseblock(int ebnum)
{
	int err = 0, i, ret = 0, eb0, ebn, rd, diff;
	loff_t  offset = 0, addr, addrn;
	unsigned int j, old_state;

	for (i = 0; i < mtd.eb_size; ++i)
		writebuf[i] = rand_r(&rnd_state);

	get_first_and_last_block(&eb0, &ebn);

	for (j = 0; j < pgcnt - 1; ++j, offset += pgsize) {
		err = flush_data_rams(eb0, ebn);
		if (err)
			return err;
		err = mtd_read(&mtd, fd, ebnum, offset, twopages, bufsize);
		if (err)
			break;
		if (memcmp(twopages, writebuf + (j * pgsize), bufsize)) {
			fprintf(stderr, "error: verify failed at block %d, page %ld\n",
					ebnum, ((long)offset) / pgsize );
			ret = -1;
		}
	}
	/* Check boundary between eraseblocks */
	addr = (loff_t)ebnum*mtd.eb_size + offset;
	addrn = (loff_t)ebn*mtd.eb_size + mtd.eb_size - 2*pgsize;

	if (addr <= addrn && !mtd_is_bad(&mtd, fd, ebnum+1)) {
		old_state = rnd_state;
		err = flush_data_rams(eb0, ebn);
		if (err)
			return err;

		if (lseek(fd, addr, SEEK_SET) != addr) {
			fprintf(stderr, "cannot seek mtd%d to offset %lld",
	 				mtd.mtd_num, (long long)addr);
			return -1;
		}

		for (rd = 0; rd < bufsize; rd += diff) {
			diff = read(fd, twopages + rd, bufsize - rd);
			if (diff < 0) {
				fprintf(stderr, "cannot read %d bytes from mtd%d "
								"(eraseblock %d, offset %d)",
								bufsize-rd, mtd.mtd_num, ebnum,
								(int)offset+rd);
				return -1;
		  	}
		}

		memcpy(boundary, writebuf + mtd.eb_size - pgsize, pgsize);

		for (j = 0; j < pgsize; ++j)
			(boundary + pgsize)[j] = rand_r(&rnd_state);

		if (memcmp(twopages, boundary, bufsize)) {
			fprintf(stderr, "error: verify failed at block %d, page %ld\n",
					ebnum, ((long)offset) / pgsize );
			ret = -1;
		}
		rnd_state = old_state;
	}
	return ret;
}

static int crosstest(void)
{
	unsigned char *pp1, *pp2, *pp3, *pp4;
	int eb0, ebn, err = 0, offset;

	puts("crosstest");
	pp1 = xzalloc(pgsize * 4);
	if (!pp1)
		return -ENOMEM;
	pp2 = pp1 + pgsize;
	pp3 = pp2 + pgsize;
	pp4 = pp3 + pgsize;

	get_first_and_last_block(&eb0, &ebn);

	/* Read 2nd-to-last page to pp1 */
	err = mtd_read(&mtd, fd, ebn, mtd.eb_size - 2*pgsize, pp1, pgsize);
	if (err)
		goto out;

	/* Read 3rd-to-last page to pp1 */
	err = mtd_read(&mtd, fd, ebn, mtd.eb_size - 3*pgsize, pp1, pgsize);
	if (err)
		goto out;

	/* Read first page to pp2 */
	printf("reading page at block %d, page %d\n", eb0, 0);
	err = mtd_read(&mtd, fd, eb0, 0, pp2, pgsize);
	if (err)
		goto out;

	/* Read last page to pp3 */
	offset = mtd.eb_size - pgsize;
	printf("reading page at block %d, page %d\n", ebn, offset/pgsize);
	err = mtd_read(&mtd, fd, ebn, offset, pp3, pgsize);
	if (err)
		goto out;

	/* Read first page again to pp4 */
	printf("reading page at block %d, page %d\n", eb0, 0);
	err = mtd_read(&mtd, fd, eb0, 0, pp4, pgsize);
	if (err)
		goto out;

	/* pp2 and pp4 should be the same */
	printf("verifying pages read at block %d match\n", eb0);
	if (memcmp(pp2, pp4, pgsize)) {
		fputs("verify failed!\n", stderr);
		err = -1;
	} else {
		puts("crosstest ok");
	}
out:
	free(pp1);
	return err;
}

static int erasecrosstest(void)
{
	unsigned char *readbuf = twopages;
	int err = 0, i, eb0, ebn;

	puts("erasecrosstest");

	get_first_and_last_block(&eb0, &ebn);

	printf("erasing block %d\n", eb0);
	err = mtd_erase(mtd_desc, &mtd, fd, eb0);
	if (err)
		return err;

	printf("writing 1st page of block %d\n", eb0);
	for (i = 0; i < pgsize; ++i)
		writebuf[i] = rand_r(&rnd_state);
	strcpy((char*)writebuf, "There is no data like this!");
	err = mtd_write(mtd_desc, &mtd, fd, eb0, 0, writebuf, pgsize, NULL, 0, 0);
	if (err)
		return err;

	printf("reading 1st page of block %d\n", eb0);
	memset(readbuf, 0, pgsize);
	err = mtd_read(&mtd, fd, eb0, 0, readbuf, pgsize);
	if (err)
		return err;

	printf("verifying 1st page of block %d\n", eb0);
	if (memcmp(writebuf, readbuf, pgsize)) {
		fputs("verify failed!\n", stderr);
		return -1;
	}

	printf("erasing block %d\n", eb0);
	err = mtd_erase(mtd_desc, &mtd, fd, eb0);
	if (err)
		return err;

	printf("writing 1st page of block %d\n", eb0);
	for (i = 0; i < pgsize; ++i)
		writebuf[i] = rand_r(&rnd_state);
	strcpy((char*)writebuf, "There is no data like this!");
	err = mtd_write(mtd_desc, &mtd, fd, eb0, 0, writebuf, pgsize, NULL, 0, 0);
	if (err)
		return err;

	printf("erasing block %d\n", ebn);
	err = mtd_erase(mtd_desc, &mtd, fd, ebn);
	if (err)
		return err;

	printf("reading 1st page of block %d\n", eb0);
	memset(readbuf, 0, pgsize);
	err = mtd_read(&mtd, fd, eb0, 0, readbuf, pgsize);
	if (err)
		return err;

	printf("verifying 1st page of block %d\n", eb0);
	if (memcmp(writebuf, readbuf, pgsize)) {
		fputs("verify failed!\n", stderr);
		return -1;
	}

	puts("erasecrosstest ok");
	return 0;
}

static int erasetest(void)
{
	int err = 0, i, ebnum, ebn;

	puts("erasetest");
	get_first_and_last_block(&ebnum, &ebn);

	printf("erasing block %d\n", ebnum);
	err = mtd_erase(mtd_desc, &mtd, fd, ebnum);
	if (err)
		return err;

	printf("writing 1st page of block %d\n", ebnum);
	for (i = 0; i < pgsize; ++i)
		writebuf[i] = rand_r(&rnd_state);
	err = mtd_write(mtd_desc, &mtd, fd, ebnum, 0,
					writebuf, pgsize, NULL, 0, 0);
	if (err)
		return err;

	printf("erasing block %d\n", ebnum);
	err = mtd_erase(mtd_desc, &mtd, fd, ebnum);
	if (err)
		return err;

	printf("reading 1st page of block %d\n", ebnum);
	err = mtd_read(&mtd, fd, ebnum, 0, twopages, pgsize);
	if (err)
		return err;

	printf("verifying 1st page of block %d is all 0xff\n", ebnum);
	for (i = 0; i < pgsize; ++i) {
		if (twopages[i] != 0xff) {
			fprintf(stderr, "verifying all 0xff failed at %d\n", i);
			return -1;
		}
	}

	puts("erasetest ok");
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

	pgsize = mtd.min_io_size;
	pgcnt = mtd.eb_size / pgsize;
	bufsize = pgsize * 2;

	if (ebcnt < 0)
		ebcnt = (mtd.eb_cnt - peb) / (skip + 1);

	if (peb >= mtd.eb_cnt)
		return errmsg("physical erase block %d is out of range!", peb);

	eb = peb + (ebcnt - 1)*(skip + 1);

	if (eb >= mtd.eb_cnt)
		return errmsg("last physical erase block %d is out of range!", eb);

	writebuf = xmalloc(mtd.eb_size);
	twopages = xmalloc(bufsize);
	boundary = xmalloc(bufsize);
	bbt = xzalloc(ebcnt);

	if ((fd = open(mtddev, O_RDWR)) == -1) {
		perror(mtddev);
		goto out_cleanup;
	}

	/* find bad blocks */
	for (i = 0; i < ebcnt; ++i) {
		eb = peb + i*(skip+1);
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

	/* Erase all eraseblocks */
	puts("erasing all blocks");
	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;
		eb = peb + i*(skip+1);
		if (mtd_erase(mtd_desc, &mtd, fd, eb)) {
			fprintf(stderr, "error erasing block %d\n", eb);
			goto out;
		}
	}
	printf("erased %u eraseblocks\n", ebcnt);

	/* Write all eraseblocks */
	rnd_state = seed;
	puts("writing all blocks");
	for (i = 0; i < ebcnt; ++i) {
		if (bbt[i])
			continue;
		eb = peb + i*(skip+1);
		err = write_eraseblock(eb);
		if (err)
			goto out;
		if (i % 256 == 0)
			printf("written up to eraseblock %u\n", i);
	}
	printf("written %u eraseblocks\n", i);

	/* Check all eraseblocks */
	rnd_state = seed;
	puts("verifying all eraseblocks");
	for (i = 0; i < ebcnt; ++i) {
		eb = peb + i*(skip+1);
		if (bbt[i])
			continue;
		err = verify_eraseblock(eb);
		if (err)
			goto out;
		if (i % 256 == 0)
			printf("verified up to eraseblock %u\n", i);
	}
	printf("verified %u eraseblocks\n", i);

	if (ebcnt > 1) {
		if (crosstest())
			goto out;
	} else {
		printf("skipping erasecrosstest, 2 erase blocks needed\n");
	}

	if (erasecrosstest())
		goto out;

	if (erasetest())
		goto out;

	status = EXIT_SUCCESS;
out:
	/* restore block backup */
	if (flags & KEEP_CONTENTS) {
		puts("restoring original contents");
		backupptr = backup;
		for (i = 0; i < ebcnt; ++i) {
			if (bbt[i])
				continue;
			eb = peb + i*(skip+1);
			if (mtd_erase(mtd_desc, &mtd, fd, eb)) {
				fprintf(stderr, "error erasing block %d!\n", eb);
				status = EXIT_FAILURE;
			}
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
	free(boundary);
	free(twopages);
	free(writebuf);
	free(backup);
	close(fd);
	return status;
}
