/* flash_erase.c -- erase MTD devices

   Copyright (C) 2000 Arcom Control System Ltd
   Copyright (C) 2010 Mike Frysinger <vapier@gentoo.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#define PROGRAM_NAME "flash_erase"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <common.h>
#include <crc32.h>
#include <libmtd.h>

#include <mtd/mtd-user.h>
#include <mtd/jffs2-user.h>

static const char *mtd_device;

static int quiet;		/* true -- don't output progress */
static int jffs2;		/* format for jffs2 usage */
static int noskipbad;		/* do not skip bad blocks */
static int unlock;		/* unlock sectors before erasing */

static struct jffs2_unknown_node cleanmarker;
int target_endian = __BYTE_ORDER;

static void show_progress(struct mtd_dev_info *mtd, off_t start, int eb,
			  int eb_start, int eb_cnt)
{
	bareverbose(!quiet, "\rErasing %d Kibyte @ %llx -- %2i %% complete ",
		mtd->eb_size / 1024, (unsigned long long)start, ((eb - eb_start) * 100) / eb_cnt);
	fflush(stdout);
}

static void display_help (void)
{
	printf("Usage: %s [options] MTD_DEVICE <start offset> <block count>\n"
			"Erase blocks of the specified MTD device.\n"
			"Specify a count of 0 to erase to end of device.\n"
			"\n"
			"  -j, --jffs2       format the device for jffs2\n"
			"  -N, --noskipbad   don't skip bad blocks\n"
			"  -u, --unlock      unlock sectors before erasing\n"
			"  -q, --quiet       do not display progress messages\n"
			"      --silent      same as --quiet\n"
			"      --help        display this help and exit\n"
			"      --version     output version information and exit\n",
			PROGRAM_NAME);
}

static void display_version (void)
{
	common_print_version();
	printf("Copyright (C) 2000 Arcom Control Systems Ltd\n"
			"\n"
			"%1$s comes with NO WARRANTY\n"
			"to the extent permitted by law.\n"
			"\n"
			"You may redistribute copies of %1$s\n"
			"under the terms of the GNU General Public Licence.\n"
			"See the file `COPYING' for more information.\n",
			PROGRAM_NAME);
}

int main(int argc, char *argv[])
{
	libmtd_t mtd_desc;
	struct mtd_dev_info mtd;
	int fd, cmlen = 8;
	unsigned long long start;
	unsigned int eb, eb_start, eb_cnt;
	bool isNAND;
	int error = 0;
	off_t offset = 0;

	/*
	 * Process user arguments
	 */
	for (;;) {
		int option_index = 0;
		static const char *short_options = "jNquVh";
		static const struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"version", no_argument, 0, 'V'},
			{"jffs2", no_argument, 0, 'j'},
			{"noskipbad", no_argument, 0, 'N'},
			{"quiet", no_argument, 0, 'q'},
			{"silent", no_argument, 0, 'q'},
			{"unlock", no_argument, 0, 'u'},

			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, short_options,
				long_options, &option_index);
		if (c == EOF)
			break;

		switch (c) {
		case 'h':
			display_help();
			return EXIT_SUCCESS;
		case 'V':
			display_version();
			return EXIT_SUCCESS;
		case 'j':
			jffs2 = 1;
			break;
		case 'N':
			noskipbad = 1;
			break;
		case 'q':
			quiet = 1;
			break;
		case 'u':
			unlock = 1;
			break;
		case '?':
			error = 1;
			break;
		}
	}
	switch (argc - optind) {
	case 3:
		mtd_device = argv[optind];
		start = simple_strtoull(argv[optind + 1], &error);
		eb_cnt = simple_strtoul(argv[optind + 2], &error);
		break;
	default:
	case 0:
		errmsg("no MTD device specified");
		/* fall-through */
	case 1:
		errmsg("no start erase block specified");
		/* fall-through */
	case 2:
		errmsg("no erase block count specified");
		error = 1;
		break;
	}
	if (error)
		return errmsg("Try `--help' for more information");

	/*
	 * Locate MTD and prepare for erasure
	 */
	mtd_desc = libmtd_open();
	if (mtd_desc == NULL)
		return errmsg("can't initialize libmtd");

	if ((fd = open(mtd_device, O_RDWR)) < 0)
		return sys_errmsg("%s", mtd_device);

	if (mtd_get_dev_info(mtd_desc, mtd_device, &mtd) < 0)
		return errmsg("mtd_get_dev_info failed");

	if (jffs2 && mtd.type == MTD_MLCNANDFLASH)
		return errmsg("JFFS2 cannot support MLC NAND.");

	eb_start = start / mtd.eb_size;

	isNAND = mtd.type == MTD_NANDFLASH || mtd.type == MTD_MLCNANDFLASH;

	if (jffs2) {
		cleanmarker.magic = cpu_to_je16 (JFFS2_MAGIC_BITMASK);
		cleanmarker.nodetype = cpu_to_je16 (JFFS2_NODETYPE_CLEANMARKER);
		if (!isNAND) {
			cleanmarker.totlen = cpu_to_je32(sizeof(cleanmarker));
		} else {
			cleanmarker.totlen = cpu_to_je32(8);
			cmlen = min(mtd.oobavail, 8);
		}
		cleanmarker.hdr_crc = cpu_to_je32(mtd_crc32(0, &cleanmarker, sizeof(cleanmarker) - 4));
	}

	/*
	 * Now do the actual erasing of the MTD device
	 */
	if (eb_cnt == 0)
		eb_cnt = (mtd.size / mtd.eb_size) - eb_start;

	for (eb = eb_start; eb < eb_start + eb_cnt; eb++) {
		offset = (off_t)eb * mtd.eb_size;

		if (!noskipbad) {
			int ret = mtd_is_bad(&mtd, fd, eb);
			if (ret > 0) {
				verbose(!quiet, "Skipping bad block at %08llx", (unsigned long long)offset);
				continue;
			} else if (ret < 0) {
				if (errno == EOPNOTSUPP) {
					noskipbad = 1;
					if (isNAND)
						return errmsg("%s: Bad block check not available", mtd_device);
				} else
					return sys_errmsg("%s: MTD get bad block failed", mtd_device);
			}
		}

		show_progress(&mtd, offset, eb, eb_start, eb_cnt);

		if (unlock) {
			if (mtd_unlock(&mtd, fd, eb) != 0) {
				sys_errmsg("%s: MTD unlock failure", mtd_device);
				continue;
			}
		}

		if (mtd_erase(mtd_desc, &mtd, fd, eb) != 0) {
			sys_errmsg("%s: MTD Erase failure", mtd_device);
			continue;
		}

		/* format for JFFS2 ? */
		if (!jffs2)
			continue;

		/* write cleanmarker */
		if (isNAND) {
			if (mtd_write(mtd_desc, &mtd, fd, eb, 0, NULL, 0, &cleanmarker, cmlen,
					MTD_OPS_AUTO_OOB) != 0) {
				sys_errmsg("%s: MTD writeoob failure", mtd_device);
				continue;
			}
		} else {
			if (pwrite(fd, &cleanmarker, sizeof(cleanmarker), (loff_t)offset) != sizeof(cleanmarker)) {
				sys_errmsg("%s: MTD write failure", mtd_device);
				continue;
			}
		}
		verbose(!quiet, " Cleanmarker Updated.");
	}
	show_progress(&mtd, offset, eb, eb_start, eb_cnt);
	bareverbose(!quiet, "\n");

	return 0;
}
