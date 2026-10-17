/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Ported 'dynenv' to 'nand env.oob' command
 * (C) 2010 Nanometrics, Inc.
 * 'dynenv' -- Dynamic environment offset in NAND OOB
 * (C) Copyright 2006-2007 OpenMoko, Inc.
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 *
 * Copyright 2010, 2012 Freescale Semiconductor
 * The portions of this file whose copyright is held by Freescale and which
 * are not considered a derived work of GPL v2-only code may be distributed
 * and/or modified under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The function nand_biterror() in this file is inspired from
 * mtd-utils/nand-utils/nandflipbits.c which was released under GPLv2
 * only
 */

#include <common.h>
#include <bootstage.h>
#include <image.h>
#include <asm/cache.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <watchdog.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>

#include "legacy-mtd-utils.h"

#if defined(CONFIG_CMD_MTDPARTS)

/* partition handling routines */
int mtdparts_init(void);
int find_dev_and_part(const char *id, struct mtd_device **dev,
		      u8 *part_num, struct part_info **part);
#endif

#define MAX_NUM_PAGES 64

static int nand_biterror(struct mtd_info *mtd, ulong off, int bit)
{
	int ret = 0;
	int page = 0;
	ulong  block_off;
	u_char *datbuf[MAX_NUM_PAGES]; /* Data and OOB */
	u_char data;
	int pages_per_blk = mtd->erasesize / mtd->writesize;
	struct erase_info einfo;

	if (pages_per_blk > MAX_NUM_PAGES) {
		printf("Too many pages in one erase block\n");
		return 1;
	}

	if (bit < 0 || bit > 7) {
		printf("bit position 0 to 7 is allowed\n");
		return 1;
	}

	/* Allocate memory */
	memset(datbuf, 0, sizeof(datbuf));
	for (page = 0; page < pages_per_blk ; page++) {
		datbuf[page] = malloc(mtd->writesize + mtd->oobsize);
		if (!datbuf[page]) {
			printf("No memory for page buffer\n");
			ret = -ENOMEM;
			goto free_memory;
		}
	}

	/* Align to erase block boundary */
	block_off = off & (~(mtd->erasesize - 1));

	/* Read out memory as first step */
	for (page = 0; page < pages_per_blk ; page++) {
		struct mtd_oob_ops ops;
		loff_t addr = (loff_t)block_off;

		memset(&ops, 0, sizeof(ops));
		ops.datbuf = datbuf[page];
		ops.oobbuf = datbuf[page] + mtd->writesize;
		ops.len = mtd->writesize;
		ops.ooblen = mtd->oobsize;
		ops.mode = MTD_OPS_RAW;
		ret = mtd_read_oob(mtd, addr, &ops);
		if (ret < 0) {
			printf("Error (%d) reading page %08lx\n",
			       ret, block_off);
			ret = 1;
			goto free_memory;
		}
		block_off += mtd->writesize;
	}

	/* Erase the block */
	memset(&einfo, 0, sizeof(einfo));
	einfo.mtd = mtd;
	/* Align to erase block boundary */
	einfo.addr = (loff_t)(off & (~(mtd->erasesize - 1)));
	einfo.len = mtd->erasesize;
	ret = mtd_erase(mtd, &einfo);
	if (ret < 0) {
		printf("Error (%d) nand_erase_nand page %08llx\n",
		       ret, einfo.addr);
		ret = 1;
		goto free_memory;
	}

	/* Twist a bit in data part */
	block_off = off & (mtd->erasesize - 1);
	data = datbuf[block_off / mtd->writesize][block_off % mtd->writesize];
	data ^= (1 << bit);
	datbuf[block_off / mtd->writesize][block_off % mtd->writesize] = data;

	printf("Flip data at 0x%lx with xor 0x%02x (bit=%d) to value=0x%02x\n",
	       off, (1 << bit), bit, data);

	/* Write back twisted data and unmodified OOB */
	/* Align to erase block boundary */
	block_off = off & (~(mtd->erasesize - 1));
	for (page = 0; page < pages_per_blk; page++) {
		struct mtd_oob_ops ops;
		loff_t addr = (loff_t)block_off;

		memset(&ops, 0, sizeof(ops));
		ops.datbuf = datbuf[page];
		ops.oobbuf = datbuf[page] + mtd->writesize;
		ops.len = mtd->writesize;
		ops.ooblen = mtd->oobsize;
		ops.mode = MTD_OPS_RAW;
		ret = mtd_write_oob(mtd, addr, &ops);
		if (ret < 0) {
			printf("Error (%d) write page %08lx\n", ret, block_off);
			ret = 1;
			goto free_memory;
		}
		block_off += mtd->writesize;
	}

free_memory:
	for (page = 0; page < pages_per_blk ; page++) {
		if (datbuf[page])
			free(datbuf[page]);
	}
	return ret;
}

static int nand_dump(struct mtd_info *mtd, ulong off, int only_oob,
		     int ecc, int repeat)
{
	int i;
	u_char *datbuf, *oobbuf, *p;
	static loff_t last;
	int ret = 0;

	if (repeat)
		off = last + mtd->writesize;

	last = off;

	datbuf = memalign(ARCH_DMA_MINALIGN, mtd->writesize);
	if (!datbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}

	oobbuf = memalign(ARCH_DMA_MINALIGN, mtd->oobsize);
	if (!oobbuf) {
		puts("No memory for page buffer\n");
		ret = 1;
		goto free_dat;
	}
	off &= ~(mtd->writesize - 1);
	loff_t addr = (loff_t) off;
	struct mtd_oob_ops ops;
	memset(&ops, 0, sizeof(ops));
	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf;
	ops.len = mtd->writesize;
	ops.ooblen = mtd->oobsize;
	if (ecc)
		ops.mode = MTD_OPS_PLACE_OOB;
	else
		ops.mode = MTD_OPS_RAW;
	i = mtd_read_oob(mtd, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page at offset %08lx, dumping raw data\n", i, off);
		ret = 1;
	}
	printf("Page at offset %08lx dump:\n", off);

	if (!only_oob) {
		i = mtd->writesize >> 4;
		p = datbuf;

		while (i--) {
			printf("\t%02x %02x %02x %02x %02x %02x %02x %02x"
			       "  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			       p[8], p[9], p[10], p[11], p[12], p[13], p[14],
			       p[15]);
			p += 16;
		}
	}

	puts("OOB:\n");
	i = mtd->oobsize >> 3;
	p = oobbuf;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}

	free(oobbuf);
free_dat:
	free(datbuf);

	return ret;
}

/* ------------------------------------------------------------------------- */

static int set_dev(int dev)
{
	struct mtd_info *mtd = get_nand_dev_by_index(dev);

	if (!mtd)
		return -ENODEV;

	if (nand_curr_device == dev)
		return 0;

	printf("Device %d: %s", dev, mtd->name);
	puts("... is now current device\n");
	nand_curr_device = dev;

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
	board_nand_select_device(mtd_to_nand(mtd), dev);
#endif

	return 0;
}

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
static void print_status(ulong start, ulong end, ulong erasesize, int status)
{
	/*
	 * Micron NAND flash (e.g. MT29F4G08ABADAH4) BLOCK LOCK READ STATUS is
	 * not the same as others.  Instead of bit 1 being lock, it is
	 * #lock_tight. To make the driver support either format, ignore bit 1
	 * and use only bit 0 and bit 2.
	 */
	printf("%08lx - %08lx: %08lx blocks %s%s%s\n",
		start,
		end - 1,
		(end - start) / erasesize,
		((status & NAND_LOCK_STATUS_TIGHT) ?  "TIGHT " : ""),
		(!(status & NAND_LOCK_STATUS_UNLOCK) ?  "LOCK " : ""),
		((status & NAND_LOCK_STATUS_UNLOCK) ?  "UNLOCK " : ""));
}

static void do_nand_status(struct mtd_info *mtd)
{
	ulong block_start = 0;
	ulong off;
	int last_status = -1;

	struct nand_chip *nand_chip = mtd_to_nand(mtd);
	/* check the WP bit */
	nand_chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	printf("device is %swrite protected\n",
		(nand_chip->read_byte(mtd) & 0x80 ?
		 "NOT " : ""));

	for (off = 0; off < mtd->size; off += mtd->erasesize) {
		int s = nand_get_lock_status(mtd, off);

		/* print message only if status has changed */
		if (s != last_status && off != 0) {
			print_status(block_start, off, mtd->erasesize,
					last_status);
			block_start = off;
		}
		last_status = s;
	}
	/* Print the last block info */
	print_status(block_start, off, mtd->erasesize, last_status);
}
#endif

#ifdef CONFIG_ENV_OFFSET_OOB
unsigned long nand_env_oob_offset;

int do_nand_env_oob(struct cmd_tbl *cmdtp, int argc, char *const argv[])
{
	int ret;
	uint32_t oob_buf[ENV_OFFSET_SIZE/sizeof(uint32_t)];
	struct mtd_info *mtd = get_nand_dev_by_index(0);
	char *cmd = argv[1];

	if (CONFIG_SYS_MAX_NAND_DEVICE == 0 || !mtd) {
		puts("no devices available\n");
		return 1;
	}

	set_dev(0);

	if (!strcmp(cmd, "get")) {
		ret = get_nand_env_oob(mtd, &nand_env_oob_offset);
		if (ret)
			return 1;

		printf("0x%08lx\n", nand_env_oob_offset);
	} else if (!strcmp(cmd, "set")) {
		loff_t addr;
		loff_t maxsize;
		struct mtd_oob_ops ops;
		int idx = 0;

		if (argc < 3)
			goto usage;

		mtd = get_nand_dev_by_index(idx);
		/* We don't care about size, or maxsize. */
		if (mtd_arg_off(argv[2], &idx, &addr, &maxsize, &maxsize,
				MTD_DEV_TYPE_NAND, mtd->size)) {
			puts("Offset or partition name expected\n");
			return 1;
		}
		if (set_dev(idx)) {
			puts("Offset or partition name expected\n");
			return 1;
		}

		if (idx != 0) {
			puts("Partition not on first NAND device\n");
			return 1;
		}

		if (mtd->oobavail < ENV_OFFSET_SIZE) {
			printf("Insufficient available OOB bytes:\n"
			       "%d OOB bytes available but %d required for "
			       "env.oob support\n",
			       mtd->oobavail, ENV_OFFSET_SIZE);
			return 1;
		}

		if ((addr & (mtd->erasesize - 1)) != 0) {
			printf("Environment offset must be block-aligned\n");
			return 1;
		}

		ops.datbuf = NULL;
		ops.mode = MTD_OPS_AUTO_OOB;
		ops.ooboffs = 0;
		ops.ooblen = ENV_OFFSET_SIZE;
		ops.oobbuf = (void *) oob_buf;

		oob_buf[0] = ENV_OOB_MARKER;
		oob_buf[1] = addr / mtd->erasesize;

		ret = mtd->write_oob(mtd, ENV_OFFSET_SIZE, &ops);
		if (ret) {
			printf("Error writing OOB block 0\n");
			return ret;
		}

		ret = get_nand_env_oob(mtd, &nand_env_oob_offset);
		if (ret) {
			printf("Error reading env offset in OOB\n");
			return ret;
		}

		if (addr != nand_env_oob_offset) {
			printf("Verification of env offset in OOB failed: "
			       "0x%08llx expected but got 0x%08lx\n",
			       (unsigned long long)addr, nand_env_oob_offset);
			return 1;
		}
	} else {
		goto usage;
	}

	return ret;

usage:
	return CMD_RET_USAGE;
}

#endif

static void nand_print_and_set_info(int idx)
{
	struct mtd_info *mtd;
	struct nand_chip *chip;

	mtd = get_nand_dev_by_index(idx);
	if (!mtd)
		return;

	chip = mtd_to_nand(mtd);
	printf("Device %d: ", idx);
	if (chip->numchips > 1)
		printf("%dx ", chip->numchips);
	printf("%s, sector size %u KiB\n",
	       mtd->name, mtd->erasesize >> 10);
	printf("  Device size 0x%llx b\n", mtd->size);
	printf("  Page size   %8d b\n", mtd->writesize);
	printf("  OOB size    %8d b\n", mtd->oobsize);
	printf("  Erase size  %8d b\n", mtd->erasesize);
	printf("  subpagesize %8d b\n", chip->subpagesize);
	printf("  ECCstrength %8d b\n", mtd->ecc_strength);
	printf("  bitfliplimit%8d b\n", mtd->bitflip_threshold);
	printf("  corrected   %8d b\n", mtd->ecc_stats.corrected);
	printf("  uncorrected %8d b\n", mtd->ecc_stats.failed);
	printf("  options       0x%08x\n", chip->options);
	printf("  bbt options   0x%08x\n", chip->bbt_options);

	/* Set geometry info */
	env_set_hex("nand_writesize", mtd->writesize);
	env_set_hex("nand_oobsize", mtd->oobsize);
	env_set_hex("nand_erasesize", mtd->erasesize);
}

static int raw_access(struct mtd_info *mtd, void *buf, loff_t off,
		      ulong count, int read, int no_verify)
{
	int ret = 0;

	while (count--) {
		/* Raw access */
		mtd_oob_ops_t ops = {
			.datbuf = buf,
			.oobbuf = buf + mtd->writesize,
			.len = mtd->writesize,
			.ooblen = mtd->oobsize,
			.mode = MTD_OPS_RAW
		};

		if (read) {
			ret = mtd_read_oob(mtd, off, &ops);
		} else {
			ret = mtd_write_oob(mtd, off, &ops);
			if (!ret && !no_verify)
				ret = nand_verify_page_oob(mtd, &ops, off);
		}

		if (ret) {
			printf("%s: error at offset %llx, ret %d\n",
				__func__, (long long)off, ret);
			break;
		}

		buf += mtd->writesize + mtd->oobsize;
		off += mtd->writesize;
	}

	return ret;
}

/* Adjust a chip/partition size down for bad blocks so we don't
 * read/write past the end of a chip/partition by accident.
 */
static void adjust_size_for_badblocks(loff_t *size, loff_t offset, int dev)
{
	/* We grab the nand info object here fresh because this is usually
	 * called after arg_off_size() which can change the value of dev.
	 */
	struct mtd_info *mtd = get_nand_dev_by_index(dev);
	loff_t maxoffset = offset + *size;
	int badblocks = 0;

	/* count badblocks in NAND from offset to offset + size */
	for (; offset < maxoffset; offset += mtd->erasesize) {
		if (nand_block_isbad(mtd, offset))
			badblocks++;
	}
	/* adjust size if any bad blocks found */
	if (badblocks) {
		*size -= badblocks * mtd->erasesize;
		printf("size adjusted to 0x%llx (%d bad blocks)\n",
		       (unsigned long long)*size, badblocks);
	}
}

#ifdef CONFIG_CMD_NAND_LOWLEVEL
#define MAX_READ_VALUES 256
#endif

static int do_nand(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	int i, ret = 0;
	ulong addr;
	loff_t off, size, maxsize;
	char *cmd, *s;
	struct mtd_info *mtd;
#ifdef CONFIG_SYS_NAND_QUIET
	int quiet = CONFIG_SYS_NAND_QUIET;
#else
	int quiet = 0;
#endif
	const char *quiet_str = env_get("quiet");
	int dev = nand_curr_device;
	int repeat = flag & CMD_FLAG_REPEAT;

	/* at least two arguments please */
	if (argc < 2)
		goto usage;

	if (quiet_str)
		quiet = simple_strtoul(quiet_str, NULL, 0) != 0;

	cmd = argv[1];

	/* Only "dump" is repeatable. */
	if (repeat && strcmp(cmd, "dump"))
		return 0;

	if (strcmp(cmd, "info") == 0) {

		putc('\n');
		for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
			nand_print_and_set_info(i);
		return 0;
	}

	if (strcmp(cmd, "device") == 0) {
		if (argc < 3) {
			putc('\n');
			if (dev < 0 || dev >= CONFIG_SYS_MAX_NAND_DEVICE)
				puts("no devices available\n");
			else
				nand_print_and_set_info(dev);
			return 0;
		}

		dev = (int)dectoul(argv[2], NULL);
		set_dev(dev);

		return 0;
	}

#ifdef CONFIG_ENV_OFFSET_OOB
	/* this command operates only on the first nand device */
	if (strcmp(cmd, "env.oob") == 0)
		return do_nand_env_oob(cmdtp, argc - 1, argv + 1);
#endif

	/* The following commands operate on the current device, unless
	 * overridden by a partition specifier.  Note that if somehow the
	 * current device is invalid, it will have to be changed to a valid
	 * one before these commands can run, even if a partition specifier
	 * for another device is to be used.
	 */
	mtd = get_nand_dev_by_index(dev);
	if (!mtd) {
		puts("\nno devices available\n");
		return 1;
	}

	if (strcmp(cmd, "bad") == 0) {
		printf("\nDevice %d bad blocks:\n", dev);
		for (off = 0; off < mtd->size; off += mtd->erasesize) {
			ret = nand_block_isbad(mtd, off);
			if (ret)
				printf("  0x%08llx%s\n", (unsigned long long)off,
				       ret == 2 ? "\t (bbt reserved)" : "");
		}
		return 0;
	}

	/*
	 * Syntax is:
	 *   0    1     2       3    4
	 *   nand erase [clean] [off size]
	 */
	if (strncmp(cmd, "erase", 5) == 0 || strncmp(cmd, "scrub", 5) == 0) {
		nand_erase_options_t opts;
		/* "clean" at index 2 means request to write cleanmarker */
		int clean = argc > 2 && !strcmp("clean", argv[2]);
		int scrub_yes = argc > 2 && !strcmp("-y", argv[2]);
		int o = (clean || scrub_yes) ? 3 : 2;
		int scrub = !strncmp(cmd, "scrub", 5);
		int spread = 0;
		int args = 2;
		const char *scrub_warn =
			"Warning: "
			"scrub option will erase all factory set bad blocks!\n"
			"         "
			"There is no reliable way to recover them.\n"
			"         "
			"Use this command only for testing purposes if you\n"
			"         "
			"are sure of what you are doing!\n"
			"\nReally scrub this NAND flash? <y/N>\n";

		if (cmd[5] != 0) {
			if (!strcmp(&cmd[5], ".spread")) {
				spread = 1;
			} else if (!strcmp(&cmd[5], ".part")) {
				args = 1;
			} else if (!strcmp(&cmd[5], ".chip")) {
				args = 0;
			} else {
				goto usage;
			}
		}

		/*
		 * Don't allow missing arguments to cause full chip/partition
		 * erases -- easy to do accidentally, e.g. with a misspelled
		 * variable name.
		 */
		if (argc != o + args)
			goto usage;

		printf("\nNAND %s: ", cmd);
		/* skip first two or three arguments, look for offset and size */
		if (mtd_arg_off_size(argc - o, argv + o, &dev, &off, &size,
				     &maxsize, MTD_DEV_TYPE_NAND,
				     mtd->size) != 0)
			return 1;

		if (set_dev(dev))
			return 1;

		mtd = get_nand_dev_by_index(dev);

		memset(&opts, 0, sizeof(opts));
		opts.offset = off;
		opts.length = size;
		opts.jffs2  = clean;
		opts.quiet  = quiet;
		opts.spread = spread;

		if (scrub) {
			if (scrub_yes) {
				opts.scrub = 1;
			} else {
				puts(scrub_warn);
				if (confirm_yesno()) {
					opts.scrub = 1;
				} else {
					puts("scrub aborted\n");
					return 1;
				}
			}
		}
		ret = nand_erase_opts(mtd, &opts);
		printf("%s\n", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
	}

	if (strncmp(cmd, "dump", 4) == 0) {
		if (argc < 3)
			goto usage;

		off = (int)hextoul(argv[2], NULL);
		ret = nand_dump(mtd, off,
				!strcmp(&cmd[4], ".oob") || !strcmp(&cmd[4], ".ecc.oob") ||
					!strcmp(&cmd[4], ".oob.ecc"),
				!strcmp(&cmd[4], ".ecc") || !strcmp(&cmd[4], ".ecc.oob") ||
					!strcmp(&cmd[4], ".oob.ecc"),
				repeat);

		return ret == 0 ? 1 : 0;
	}

	if (strncmp(cmd, "read", 4) == 0 || strncmp(cmd, "write", 5) == 0) {
		size_t rwsize;
		ulong pagecount = 1;
		int read;
		int raw = 0;
		int no_verify = 0;
		void *buf;

		if (argc < 4)
			goto usage;

		addr = (ulong)hextoul(argv[2], NULL);

		read = strncmp(cmd, "read", 4) == 0; /* 1 = read, 0 = write */
		printf("\nNAND %s: ", read ? "read" : "write");

		s = strchr(cmd, '.');

		if (s && !strncmp(s, ".raw", 4)) {
			raw = 1;

			if (!strcmp(s, ".raw.noverify"))
				no_verify = 1;

			if (mtd_arg_off(argv[3], &dev, &off, &size, &maxsize,
					MTD_DEV_TYPE_NAND,
					mtd->size))
				return 1;

			if (set_dev(dev))
				return 1;

			mtd = get_nand_dev_by_index(dev);

			if (argc > 4 && !str2long(argv[4], &pagecount)) {
				printf("'%s' is not a number\n", argv[4]);
				return 1;
			}

			if (pagecount * mtd->writesize > size) {
				puts("Size exceeds partition or device limit\n");
				return -1;
			}

			rwsize = pagecount * (mtd->writesize + mtd->oobsize);
		} else {
			if (mtd_arg_off_size(argc - 3, argv + 3, &dev, &off,
					     &size, &maxsize,
					     MTD_DEV_TYPE_NAND,
					     mtd->size) != 0)
				return 1;

			if (set_dev(dev))
				return 1;

			/* size is unspecified */
			if (argc < 5)
				adjust_size_for_badblocks(&size, off, dev);
			rwsize = size;
		}

		mtd = get_nand_dev_by_index(dev);
		buf = map_sysmem(addr, maxsize);

		if (!s || !strcmp(s, ".jffs2") ||
		    !strcmp(s, ".e") || !strcmp(s, ".i")) {
			if (read)
				ret = nand_read_skip_bad(mtd, off, &rwsize,
							 NULL, maxsize, buf);
			else
				ret = nand_write_skip_bad(mtd, off, &rwsize,
							  NULL, maxsize, buf,
							  WITH_WR_VERIFY);
#ifdef CONFIG_CMD_NAND_TRIMFFS
		} else if (!strcmp(s, ".trimffs")) {
			if (read) {
				printf("Unknown nand command suffix '%s'\n", s);
				unmap_sysmem(buf);
				return 1;
			}
			ret = nand_write_skip_bad(mtd, off, &rwsize, NULL,
						maxsize, buf,
						WITH_DROP_FFS | WITH_WR_VERIFY);
#endif
		} else if (!strcmp(s, ".oob")) {
			/* out-of-band data */
			mtd_oob_ops_t ops = {
				.oobbuf = buf,
				.ooblen = rwsize,
				.mode = MTD_OPS_RAW
			};

			if (read)
				ret = mtd_read_oob(mtd, off, &ops);
			else
				ret = mtd_write_oob(mtd, off, &ops);
		} else if (raw) {
			ret = raw_access(mtd, buf, off, pagecount, read,
					 no_verify);
		} else {
			printf("Unknown nand command suffix '%s'.\n", s);
			unmap_sysmem(buf);
			return 1;
		}

		unmap_sysmem(buf);
		printf(" %zu bytes %s: %s\n", rwsize,
		       read ? "read" : "written", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
	}

#ifdef CONFIG_CMD_NAND_TORTURE
	if (strcmp(cmd, "torture") == 0) {
		loff_t endoff;
		unsigned int failed = 0, passed = 0;

		if (argc < 3)
			goto usage;

		if (!str2off(argv[2], &off)) {
			puts("Offset is not a valid number\n");
			return 1;
		}

		size = mtd->erasesize;
		if (argc > 3) {
			if (!str2off(argv[3], &size)) {
				puts("Size is not a valid number\n");
				return 1;
			}
		}

		endoff = off + size;
		if (endoff > mtd->size) {
			puts("Arguments beyond end of NAND\n");
			return 1;
		}

		off = round_down(off, mtd->erasesize);
		endoff = round_up(endoff, mtd->erasesize);
		size = endoff - off;
		printf("\nNAND torture: device %d offset 0x%llx size 0x%llx (block size 0x%x)\n",
		       dev, off, size, mtd->erasesize);
		while (off < endoff) {
			ret = nand_torture(mtd, off);
			if (ret) {
				failed++;
				printf("  block at 0x%llx failed\n", off);
			} else {
				passed++;
			}
			off += mtd->erasesize;
		}
		printf(" Passed: %u, failed: %u\n", passed, failed);
		return failed != 0;
	}
#endif

	if (strcmp(cmd, "markbad") == 0) {
		argc -= 2;
		argv += 2;

		if (argc <= 0)
			goto usage;

		while (argc > 0) {
			addr = hextoul(*argv, NULL);

			if (mtd_block_markbad(mtd, addr)) {
				printf("block 0x%08lx NOT marked "
					"as bad! ERROR %d\n",
					addr, ret);
				ret = 1;
			} else {
				printf("block 0x%08lx successfully "
					"marked as bad\n",
					addr);
			}
			--argc;
			++argv;
		}
		return ret;
	}

	if (strcmp(cmd, "biterr") == 0) {
		int bit;

		if (argc != 4)
			goto usage;

		off = (int)simple_strtoul(argv[2], NULL, 16);
		bit = (int)simple_strtoul(argv[3], NULL, 10);
		ret = nand_biterror(mtd, off, bit);
		return ret;
	}

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
	if (strcmp(cmd, "lock") == 0) {
		int tight = 0;
		int status = 0;
		if (argc == 3) {
			if (!strcmp("tight", argv[2]))
				tight = 1;
			if (!strcmp("status", argv[2]))
				status = 1;
		}
		if (status) {
			do_nand_status(mtd);
		} else {
			if (!nand_lock(mtd, tight)) {
				puts("NAND flash successfully locked\n");
			} else {
				puts("Error locking NAND flash\n");
				return 1;
			}
		}
		return 0;
	}

	if (strncmp(cmd, "unlock", 5) == 0) {
		int allexcept = 0;

		s = strchr(cmd, '.');

		if (s && !strcmp(s, ".allexcept"))
			allexcept = 1;

		if (mtd_arg_off_size(argc - 2, argv + 2, &dev, &off, &size,
				     &maxsize, MTD_DEV_TYPE_NAND,
				     mtd->size) < 0)
			return 1;

		if (set_dev(dev))
			return 1;

		mtd = get_nand_dev_by_index(dev);

		if (!nand_unlock(mtd, off, size, allexcept)) {
			puts("NAND flash successfully unlocked\n");
		} else {
			puts("Error unlocking NAND flash, "
			     "write and erase will probably fail\n");
			return 1;
		}
		return 0;
	}
#endif
#ifdef CONFIG_CMD_NAND_LOWLEVEL
	if (strncmp(cmd, "cmd", 3) == 0) {
		int item = 2, read = 0;
		unsigned char data[MAX_READ_VALUES];

		argc -= 2;

		if (argc <= 0)
			goto usage;

		bool spi_nand = !strncmp(mtd->dev->name, "spi-nand", 8);

		if (spi_nand) { // SPI NAND
			int write = 0;

			while (argc--) { // ignore superfluous commands
				if ((argv[item][0] == 'c') || (argv[item][0] == 'a') ||
				    (argv[item][0] == 'w') ||
				    (argv[item][0] == 'C') || (argv[item][0] == 'A') ||
				    (argv[item][0] == 'W')) {
					item++;
					continue;
				}

				/* if a read is requested, get the amount and stop
				 * since we can't support a command any more compound
				 */
				if ((argv[item][0] == 'r') || (argv[item][0] == 'R')) {
					read = simple_strtoul(argv[++item], NULL, 16);

					break;
				}

				// otherwise get the write data
				data[write++] = simple_strtoul(argv[item++], NULL, 16);
			}

			mtd_read_user_prot_reg(mtd, write, read, 0, data);
		} else { // parallel NAND
			u16 value;
			char command = 0, address;

			register struct nand_chip *chip = mtd_to_nand(mtd);

			/* select the NAND device */
			chip->select_chip(mtd, 0);

			while (argc-- > 0) {
				if ((argv[item][0] == 'c') || (argv[item][0] == 'C')) {
					command = simple_strtoul(argv[++item], NULL, 16);
					argc--;
				}

				if ((argv[item][0] == 'a') || (argv[item][0] == 'A')) {
					address = simple_strtoul(argv[++item], NULL, 16);
					argc--;
					chip->cmdfunc(mtd, command, address, -1);
				}

				if ((argv[item][0] == 'r') || (argv[item][0] == 'R')) {
					value = simple_strtoul(argv[++item], NULL, 16);
					argc--;

					if (value > MAX_READ_VALUES)
						value = MAX_READ_VALUES;

					while (value--)
						data[read++] = chip->read_byte(mtd);
				}

				if ((argv[item][0] == 'w') || (argv[item][0] == 'W')) {
					argc--;

					while (argc-- > 0)
						chip->write_byte(mtd, simple_strtoul(argv[++item],
							NULL, 16));
				}

				item++;
			}

			chip->select_chip(mtd, -1);

			item = 0;
		}

		if (read) {
			printf("read data: ");

			item = 0;

			while (read--)
				printf("0x%x ", data[item++]);

			printf("\n");
		}

		return(0);
	}
#endif
usage:
	return CMD_RET_USAGE;
}

U_BOOT_LONGHELP(nand,
	"info - show available NAND devices\n"
	"nand device [dev] - show or set current device\n"
	"nand read - addr off|partition size\n"
	"nand write - addr off|partition size\n"
	"    read/write 'size' bytes starting at offset 'off'\n"
	"    to/from memory address 'addr', skipping bad blocks.\n"
	"nand read.raw - addr off|partition [pages]\n"
	"nand write.raw[.noverify] - addr off|partition [pages]\n"
	"    Use read.raw/write.raw to avoid ECC and access the flash as-is.\n"
#ifdef CONFIG_CMD_NAND_TRIMFFS
	"nand write.trimffs - addr off|partition size\n"
	"    write 'size' bytes starting at offset 'off' from memory address\n"
	"    'addr', skipping bad blocks and dropping any pages at the end\n"
	"    of eraseblocks that contain only 0xFF\n"
#endif
	"nand erase[.spread] [clean] off size - erase 'size' bytes "
	"from offset 'off'\n"
	"    With '.spread', erase enough for given file size, otherwise,\n"
	"    'size' includes skipped bad blocks.\n"
	"nand erase.part [clean] partition - erase entire mtd partition'\n"
	"nand erase.chip [clean] - erase entire chip'\n"
	"nand bad - show bad blocks\n"
	"nand dump[.oob][.ecc] off - dump raw (default) or ecc corrected page at offset\n"
#ifdef CONFIG_CMD_NAND_TORTURE
	"nand torture off - torture one block at offset\n"
	"nand torture off [size] - torture blocks from off to off+size\n"
#endif
	"nand scrub [-y] off size | scrub.part partition | scrub.chip\n"
	"    really clean NAND erasing bad blocks (UNSAFE)\n"
	"nand markbad off [...] - mark bad block(s) at offset (UNSAFE)\n"
	"nand biterr off bit - make a bit error at offset and bit position (UNSAFE)"
#ifdef CONFIG_CMD_NAND_LOWLEVEL
	"\nnand cmd [...] - issue a command to the NAND\n"
	"    a [address byte], c [command byte], w [write byte values], r [read # of bytes], examples:\n"
	"    parallel NAND:\n"
	"       to get feature drive strength: nand cmd c 0xEE a 0x80 r 4\n"
	"       to set feature drive strength: nand cmd c 0xEF a 0x80 w 1 0 0 0\n"
	"       to read device ID:             nand cmd c 0x90 a 0 r 4\n"
	"       to read parameter page:        nand cmd c 0xEC a 0 r 8\n"
	"       read parameter page byte 100:  nand cmd c 0xEC a 0 c 5 a 100 0 c 0xE0 r 1\n\n"
	"    serial SPI NAND:\n"
	"       get device ID:                 nand cmd [w] 0x9F 0 r 8\n"
	"       read config (get feature):     nand cmd [w] 0xF 0xB0 r 1\n"
	"       read config (set feature):     nand cmd [w] 0x1F 0xB0 0xAB\n"
	"       read status (get feature):     nand cmd [w] 0xF 0xC0 r 1\n"
	"       reset part:                    nand cmd [w] 0xFF\n"
#endif
#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
	"\n"
	"nand lock [tight] [status]\n"
	"    bring nand to lock state or display locked pages\n"
	"nand unlock[.allexcept] [offset] [size] - unlock section"
#endif
#ifdef CONFIG_ENV_OFFSET_OOB
	"\n"
	"nand env.oob - environment offset in OOB of block 0 of"
	"    first device.\n"
	"nand env.oob set off|partition - set enviromnent offset\n"
	"nand env.oob get - get environment offset"
#endif
	);

U_BOOT_CMD(
	nand, CONFIG_SYS_MAXARGS, 1, do_nand,
	"NAND sub-system", nand_help_text
);

static int nand_load_image(struct cmd_tbl *cmdtp, struct mtd_info *mtd,
			   ulong offset, ulong addr, char *cmd)
{
	int r;
	char *s;
	size_t cnt;
#if defined(CONFIG_LEGACY_IMAGE_FORMAT)
	struct legacy_img_hdr *hdr;
#endif
#if defined(CONFIG_FIT)
	const void *fit_hdr = NULL;
#endif

	s = strchr(cmd, '.');
	if (s != NULL &&
	    (strcmp(s, ".jffs2") && strcmp(s, ".e") && strcmp(s, ".i"))) {
		printf("Unknown nand load suffix '%s'\n", s);
		bootstage_error(BOOTSTAGE_ID_NAND_SUFFIX);
		return 1;
	}

	printf("\nLoading from %s, offset 0x%lx\n", mtd->name, offset);

	cnt = mtd->writesize;
	r = nand_read_skip_bad(mtd, offset, &cnt, NULL, mtd->size,
			       (u_char *)addr);
	if (r) {
		puts("** Read error\n");
		bootstage_error(BOOTSTAGE_ID_NAND_HDR_READ);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_NAND_HDR_READ);

	switch (genimg_get_format ((void *)addr)) {
#if defined(CONFIG_LEGACY_IMAGE_FORMAT)
	case IMAGE_FORMAT_LEGACY:
		hdr = (struct legacy_img_hdr *)addr;

		bootstage_mark(BOOTSTAGE_ID_NAND_TYPE);
		image_print_contents (hdr);

		cnt = image_get_image_size (hdr);
		break;
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = (const void *)addr;
		puts ("Fit image detected...\n");

		cnt = fit_get_size (fit_hdr);
		break;
#endif
	default:
		bootstage_error(BOOTSTAGE_ID_NAND_TYPE);
		puts ("** Unknown image type\n");
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_NAND_TYPE);

	r = nand_read_skip_bad(mtd, offset, &cnt, NULL, mtd->size,
			       (u_char *)addr);
	if (r) {
		puts("** Read error\n");
		bootstage_error(BOOTSTAGE_ID_NAND_READ);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_NAND_READ);

#if defined(CONFIG_FIT)
	/* This cannot be done earlier, we need complete FIT image in RAM first */
	if (genimg_get_format ((void *)addr) == IMAGE_FORMAT_FIT) {
		if (fit_check_format(fit_hdr, IMAGE_SIZE_INVAL)) {
			bootstage_error(BOOTSTAGE_ID_NAND_FIT_READ);
			puts ("** Bad FIT image format\n");
			return 1;
		}
		bootstage_mark(BOOTSTAGE_ID_NAND_FIT_READ_OK);
		fit_print_contents (fit_hdr);
	}
#endif

	/* Loading ok, update default load address */

	image_load_addr = addr;

	return bootm_maybe_autostart(cmdtp, cmd);
}

static int do_nandboot(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	char *boot_device = NULL;
	int idx;
	ulong addr, offset = 0;
	struct mtd_info *mtd;
#if defined(CONFIG_CMD_MTDPARTS)
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;

	if (argc >= 2) {
		char *p = (argc == 2) ? argv[1] : argv[2];
		if (!(str2long(p, &addr)) && (mtdparts_init() == 0) &&
		    (find_dev_and_part(p, &dev, &pnum, &part) == 0)) {
			if (dev->id->type != MTD_DEV_TYPE_NAND) {
				puts("Not a NAND device\n");
				return 1;
			}
			if (argc > 3)
				goto usage;
			if (argc == 3)
				addr = hextoul(argv[1], NULL);
			else
				addr = CONFIG_SYS_LOAD_ADDR;

			mtd = get_nand_dev_by_index(dev->id->num);
			return nand_load_image(cmdtp, mtd, part->offset,
					       addr, argv[0]);
		}
	}
#endif

	bootstage_mark(BOOTSTAGE_ID_NAND_PART);
	switch (argc) {
	case 1:
		addr = CONFIG_SYS_LOAD_ADDR;
		boot_device = env_get("bootdevice");
		break;
	case 2:
		addr = hextoul(argv[1], NULL);
		boot_device = env_get("bootdevice");
		break;
	case 3:
		addr = hextoul(argv[1], NULL);
		boot_device = argv[2];
		break;
	case 4:
		addr = hextoul(argv[1], NULL);
		boot_device = argv[2];
		offset = hextoul(argv[3], NULL);
		break;
	default:
#if defined(CONFIG_CMD_MTDPARTS)
usage:
#endif
		bootstage_error(BOOTSTAGE_ID_NAND_SUFFIX);
		return CMD_RET_USAGE;
	}
	bootstage_mark(BOOTSTAGE_ID_NAND_SUFFIX);

	if (!boot_device) {
		puts("\n** No boot device **\n");
		bootstage_error(BOOTSTAGE_ID_NAND_BOOT_DEVICE);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_NAND_BOOT_DEVICE);

	idx = hextoul(boot_device, NULL);

	mtd = get_nand_dev_by_index(idx);
	if (!mtd) {
		printf("\n** Device %d not available\n", idx);
		bootstage_error(BOOTSTAGE_ID_NAND_AVAILABLE);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_NAND_AVAILABLE);

	return nand_load_image(cmdtp, mtd, offset, addr, argv[0]);
}

U_BOOT_CMD(nboot, 4, 1, do_nandboot,
	"boot from NAND device",
	"[partition] | [[[loadAddr] dev] offset]"
);
