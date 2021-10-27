/*
 * Copyright (C) 2006-2008 Artem Bityutskiy
 * Copyright (C) 2006-2008 Jarkko Lavinen
 * Copyright (C) 2006-2008 Adrian Hunter
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
 *
 * WARNING: this test program may kill your flash and your device. Do not
 * use it unless you know what you do. Authors are not responsible for any
 * damage caused by this program.
 *
 * Author: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 *
 * Based on linux torturetest.c
 * Authors: Artem Bityutskiy, Jarkko Lavinen, Adria Hunter
 */

#define PROGRAM_NAME "flash_torture"

#define KEEP_CONTENTS 0x01
#define RUN_FOREVER 0x02

#include <mtd/mtd-user.h>
#include <unistd.h>
#include <stdlib.h>
#include <libmtd.h>
#include <signal.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>

#include "common.h"

static int peb=-1, blocks=-1, skip=-1;
static struct mtd_dev_info mtd;
static sig_atomic_t flags=0;
static const char *mtddev;
static libmtd_t mtd_desc;
static int mtdfd;

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "peb", required_argument, NULL, 'b' },
	{ "count", required_argument, NULL, 'c' },
	{ "skip", required_argument, NULL, 's' },
	{ "keep", no_argument, NULL, 'k' },
	{ "repeate", no_argument, NULL, 'r' },
	{ NULL, 0, NULL, 0 },
};

static void sighandler(int sig)
{
	if (sig == SIGINT || sig == SIGTERM || sig == SIGHUP)
		flags &= ~RUN_FOREVER;
}

static NORETURN void usage(int status)
{
	fputs(
	"Usage: "PROGRAM_NAME" [OPTIONS] <device>\n\n"
	"Options:\n"
	"  -h, --help         Display this help output\n"
	"  -b, --peb <num>    Start from this physical erase block\n"
	"  -c, --count <num>  Number of erase blocks to torture\n"
	"  -s, --skip <num>   Number of erase blocks to skip\n"
	"  -k, --keep         Try to restore existing contents after test\n"
	"  -r, --repeate      Repeate the torture test indefinitely\n",
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
		c = getopt_long(argc, argv, "hb:c:s:kr", options, NULL);
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
			if (blocks > 0)
				goto failmulti;
			blocks = read_num(c, optarg);
			if (blocks <= 0)
				goto failarg;
			break;
		case 's':
			if (skip >= 0)
				goto failmulti;
			skip = read_num(c, optarg);
			if (skip < 0)
				goto failarg;
			break;
		case 'k':
			if (flags & KEEP_CONTENTS)
				goto failmulti;
			flags |= KEEP_CONTENTS;
			break;
		case 'r':
			if (flags & RUN_FOREVER)
				goto failmulti;
			flags |= RUN_FOREVER;
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
	if (blocks < 0)
		blocks = 1;
	return;
failmulti:
	errmsg_die("'-%c' specified more than once!\n", c);
failarg:
	errmsg_die("Invalid argument for '-%c'!\n", c);
}

int main(int argc, char **argv)
{
	int i, eb, err, count = 0;
	char *is_bad = NULL;
	void *old=NULL;

	process_options(argc, argv);

	mtd_desc = libmtd_open();
	if (!mtd_desc)
		return errmsg("can't initialize libmtd");

	if (mtd_get_dev_info(mtd_desc, mtddev, &mtd) < 0)
		return errmsg("mtd_get_dev_info failed");

	if (peb >= mtd.eb_cnt)
		return errmsg("Physical erase block %d is out of range!\n", peb);

	if ((peb + (blocks - 1)*(skip + 1)) >= mtd.eb_cnt)
		return errmsg("Given block range exceeds block count of %d!\n",
				mtd.eb_cnt);

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGHUP, sighandler);

	if (flags & KEEP_CONTENTS)
		old = xmalloc(mtd.eb_size);

	is_bad = xmalloc(blocks);

	if ((mtdfd = open(mtddev, O_RDWR)) == -1) {
		perror(mtddev);
		free(is_bad);
		free(old);
		return EXIT_FAILURE;
	}

	for (i = 0; i < blocks; ++i) {
		eb = peb + i * (skip + 1);
		is_bad[i] = mtd_is_bad(&mtd, mtdfd, eb);
		if (is_bad[i])
			fprintf(stderr, "PEB %d marked bad, will be skipped\n", eb);
	}

	do {
		for (i = 0; i < blocks; ++i) {
			if (is_bad[i])
				continue;

			eb = peb + i * (skip + 1);

			if (flags & KEEP_CONTENTS) {
				err = mtd_read(&mtd, mtdfd, eb, 0, old, mtd.eb_size);
				if (err) {
					fprintf(stderr, "Failed to create backup copy "
							"of PEB %d, skipping!\n", eb);
					continue;
				}
			}

			if (mtd_torture(mtd_desc, &mtd, mtdfd, eb))
				fprintf(stderr, "Block %d failed torture test!\n", eb);

			if (flags & KEEP_CONTENTS) {
				err = mtd_erase(mtd_desc, &mtd, mtdfd, eb);
				if (err) {
					fprintf(stderr, "mtd_erase failed for block %d!\n", eb);
					continue;
				}
				err = mtd_write(mtd_desc, &mtd, mtdfd, eb, 0,
						old, mtd.eb_size, NULL, 0, 0);
				if (err)
					fprintf(stderr, "Failed to restore block %d!\n", eb);
			}
		}

		printf("Torture test iterations done: %d\n", ++count);
	} while (flags & RUN_FOREVER);

	free(old);
	free(is_bad);
	close(mtdfd);
	return EXIT_SUCCESS;
}

