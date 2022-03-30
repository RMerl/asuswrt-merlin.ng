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
 */

#include <common.h>
#include <linux/mtd/mtd.h>
#include <command.h>
#include <console.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>

#if defined(CONFIG_CMD_MTDPARTS)

/* partition handling routines */
int mtdparts_init(void);
int id_parse(const char *id, const char **ret_id, u8 *dev_type, u8 *dev_num);
int find_dev_and_part(const char *id, struct mtd_device **dev,
		      u8 *part_num, struct part_info **part);
#endif

static int nand_dump(struct mtd_info *mtd, ulong off, int only_oob,
		     int repeat)
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
	ops.mode = MTD_OPS_RAW;
	i = mtd_read_oob(mtd, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		ret = 1;
		goto free_all;
	}
	printf("Page %08lx dump:\n", off);

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

free_all:
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

int do_nand_env_oob(cmd_tbl_t *cmdtp, int argc, char *const argv[])
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
		ops.mode = MTD_OOB_AUTO;
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
	printf("  Page size   %8d b\n", mtd->writesize);
	printf("  OOB size    %8d b\n", mtd->oobsize);
	printf("  Erase size  %8d b\n", mtd->erasesize);
	printf("  subpagesize %8d b\n", chip->subpagesize);
	printf("  options     0x%08x\n", chip->options);
	printf("  bbt options 0x%08x\n", chip->bbt_options);

	/* Set geometry info */
	env_set_hex("nand_writesize", mtd->writesize);
	env_set_hex("nand_oobsize", mtd->oobsize);
	env_set_hex("nand_erasesize", mtd->erasesize);
}

static int raw_access(struct mtd_info *mtd, ulong addr, loff_t off,
		      ulong count, int read, int no_verify)
{
	int ret = 0;

	while (count--) {
		/* Raw access */
		mtd_oob_ops_t ops = {
			.datbuf = (u8 *)addr,
			.oobbuf = ((u8 *)addr) + mtd->writesize,
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

		addr += mtd->writesize + mtd->oobsize;
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

static int do_nand(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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

		dev = (int)simple_strtoul(argv[2], NULL, 10);
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
		for (off = 0; off < mtd->size; off += mtd->erasesize)
			if (nand_block_isbad(mtd, off))
				printf("  %08llx\n", (unsigned long long)off);
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

		off = (int)simple_strtoul(argv[2], NULL, 16);
		ret = nand_dump(mtd, off, !strcmp(&cmd[4], ".oob"), repeat);

		return ret == 0 ? 1 : 0;
	}

	if (strncmp(cmd, "read", 4) == 0 || strncmp(cmd, "write", 5) == 0) {
		size_t rwsize;
		ulong pagecount = 1;
		int read;
		int raw = 0;
		int no_verify = 0;

		if (argc < 4)
			goto usage;

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);

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

		if (!s || !strcmp(s, ".jffs2") ||
		    !strcmp(s, ".e") || !strcmp(s, ".i")) {
			if (read)
				ret = nand_read_skip_bad(mtd, off, &rwsize,
							 NULL, maxsize,
							 (u_char *)addr);
			else
				ret = nand_write_skip_bad(mtd, off, &rwsize,
							  NULL, maxsize,
							  (u_char *)addr,
							  WITH_WR_VERIFY);
#ifdef CONFIG_CMD_NAND_TRIMFFS
		} else if (!strcmp(s, ".trimffs")) {
			if (read) {
				printf("Unknown nand command suffix '%s'\n", s);
				return 1;
			}
			ret = nand_write_skip_bad(mtd, off, &rwsize, NULL,
						maxsize, (u_char *)addr,
						WITH_DROP_FFS | WITH_WR_VERIFY);
#endif
		} else if (!strcmp(s, ".oob")) {
			/* out-of-band data */
			mtd_oob_ops_t ops = {
				.oobbuf = (u8 *)addr,
				.ooblen = rwsize,
				.mode = MTD_OPS_RAW
			};

			if (read)
				ret = mtd_read_oob(mtd, off, &ops);
			else
				ret = mtd_write_oob(mtd, off, &ops);
		} else if (raw) {
			ret = raw_access(mtd, addr, off, pagecount, read,
					 no_verify);
		} else {
			printf("Unknown nand command suffix '%s'.\n", s);
			return 1;
		}

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
			addr = simple_strtoul(*argv, NULL, 16);

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
		/* todo */
		return 1;
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

usage:
	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char nand_help_text[] =
	"info - show available NAND devices\n"
	"nand device [dev] - show or set current device\n"
	"nand read - addr off|partition size\n"
	"nand write - addr off|partition size\n"
	"    read/write 'size' bytes starting at offset 'off'\n"
	"    to/from memory address 'addr', skipping bad blocks.\n"
	"nand read.raw - addr off|partition [count]\n"
	"nand write.raw[.noverify] - addr off|partition [count]\n"
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
	"nand dump[.oob] off - dump page\n"
#ifdef CONFIG_CMD_NAND_TORTURE
	"nand torture off - torture one block at offset\n"
	"nand torture off [size] - torture blocks from off to off+size\n"
#endif
	"nand scrub [-y] off size | scrub.part partition | scrub.chip\n"
	"    really clean NAND erasing bad blocks (UNSAFE)\n"
	"nand markbad off [...] - mark bad block(s) at offset (UNSAFE)\n"
	"nand biterr off - make a bit error at offset (UNSAFE)"
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
	"";
#endif

U_BOOT_CMD(
	nand, CONFIG_SYS_MAXARGS, 1, do_nand,
	"NAND sub-system", nand_help_text
);

static int nand_load_image(cmd_tbl_t *cmdtp, struct mtd_info *mtd,
			   ulong offset, ulong addr, char *cmd)
{
	int r;
	char *s;
	size_t cnt;
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	image_header_t *hdr;
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
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	case IMAGE_FORMAT_LEGACY:
		hdr = (image_header_t *)addr;

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
		if (!fit_check_format (fit_hdr)) {
			bootstage_error(BOOTSTAGE_ID_NAND_FIT_READ);
			puts ("** Bad FIT image format\n");
			return 1;
		}
		bootstage_mark(BOOTSTAGE_ID_NAND_FIT_READ_OK);
		fit_print_contents (fit_hdr);
	}
#endif

	/* Loading ok, update default load address */

	load_addr = addr;

	return bootm_maybe_autostart(cmdtp, cmd);
}

static int do_nandboot(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
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
				addr = simple_strtoul(argv[1], NULL, 16);
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
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = env_get("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	case 4:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		offset = simple_strtoul(argv[3], NULL, 16);
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

	idx = simple_strtoul(boot_device, NULL, 16);

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
