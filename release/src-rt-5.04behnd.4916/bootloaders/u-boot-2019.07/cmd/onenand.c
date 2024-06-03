/*
 *  U-Boot command for OneNAND support
 *
 *  Copyright (C) 2005-2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>

#include <linux/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <asm/io.h>

static struct mtd_info *mtd;

static loff_t next_ofs;
static loff_t skip_ofs;

static int arg_off_size_onenand(int argc, char * const argv[], ulong *off,
				size_t *size)
{
	if (argc >= 1) {
		if (!(str2long(argv[0], off))) {
			printf("'%s' is not a number\n", argv[0]);
			return -1;
		}
	} else {
		*off = 0;
	}

	if (argc >= 2) {
		if (!(str2long(argv[1], (ulong *)size))) {
			printf("'%s' is not a number\n", argv[1]);
			return -1;
		}
	} else {
		*size = mtd->size - *off;
	}

	if ((*off + *size) > mtd->size) {
		printf("total chip size (0x%llx) exceeded!\n", mtd->size);
		return -1;
	}

	if (*size == mtd->size)
		puts("whole chip\n");
	else
		printf("offset 0x%lx, size 0x%x\n", *off, *size);

	return 0;
}

static int onenand_block_read(loff_t from, size_t len,
			      size_t *retlen, u_char *buf, int oob)
{
	struct onenand_chip *this = mtd->priv;
	int blocks = (int) len >> this->erase_shift;
	int blocksize = (1 << this->erase_shift);
	loff_t ofs = from;
	struct mtd_oob_ops ops = {
		.retlen		= 0,
	};
	int ret;

	if (oob)
		ops.ooblen = blocksize;
	else
		ops.len = blocksize;

	while (blocks) {
		ret = mtd_block_isbad(mtd, ofs);
		if (ret) {
			printk("Bad blocks %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			ofs += blocksize;
			continue;
		}

		if (oob)
			ops.oobbuf = buf;
		else
			ops.datbuf = buf;

		ops.retlen = 0;
		ret = mtd_read_oob(mtd, ofs, &ops);
		if (ret) {
			printk("Read failed 0x%x, %d\n", (u32)ofs, ret);
			ofs += blocksize;
			continue;
		}
		ofs += blocksize;
		buf += blocksize;
		blocks--;
		*retlen += ops.retlen;
	}

	return 0;
}

static int onenand_write_oneblock_withoob(loff_t to, const u_char * buf,
					  size_t *retlen)
{
	struct mtd_oob_ops ops = {
		.len = mtd->writesize,
		.ooblen = mtd->oobsize,
		.mode = MTD_OPS_AUTO_OOB,
	};
	int page, ret = 0;
	for (page = 0; page < (mtd->erasesize / mtd->writesize); page ++) {
		ops.datbuf = (u_char *)buf;
		buf += mtd->writesize;
		ops.oobbuf = (u_char *)buf;
		buf += mtd->oobsize;
		ret = mtd_write_oob(mtd, to, &ops);
		if (ret)
			break;
		to += mtd->writesize;
	}

	*retlen = (ret) ? 0 : mtd->erasesize;
	return ret;
}

static int onenand_block_write(loff_t to, size_t len,
			       size_t *retlen, const u_char * buf, int withoob)
{
	struct onenand_chip *this = mtd->priv;
	int blocks = len >> this->erase_shift;
	int blocksize = (1 << this->erase_shift);
	loff_t ofs;
	size_t _retlen = 0;
	int ret;

	if ((to & (mtd->writesize - 1)) != 0) {
		printf("Attempt to write non block-aligned data\n");
		*retlen = 0;
		return 1;
	}

	if (to == next_ofs) {
		next_ofs = to + len;
		to += skip_ofs;
	} else {
		next_ofs = to + len;
		skip_ofs = 0;
	}
	ofs = to;

	while (blocks) {
		ret = mtd_block_isbad(mtd, ofs);
		if (ret) {
			printk("Bad blocks %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			skip_ofs += blocksize;
			goto next;
		}

		if (!withoob)
			ret = mtd_write(mtd, ofs, blocksize, &_retlen, buf);
		else
			ret = onenand_write_oneblock_withoob(ofs, buf, &_retlen);
		if (ret) {
			printk("Write failed 0x%x, %d", (u32)ofs, ret);
			skip_ofs += blocksize;
			goto next;
		}

		buf += blocksize;
		blocks--;
		*retlen += _retlen;
next:
		ofs += blocksize;
	}

	return 0;
}

static int onenand_block_erase(u32 start, u32 size, int force)
{
	struct onenand_chip *this = mtd->priv;
	struct erase_info instr = {
		.callback	= NULL,
	};
	loff_t ofs;
	int ret;
	int blocksize = 1 << this->erase_shift;

	for (ofs = start; ofs < (start + size); ofs += blocksize) {
		ret = mtd_block_isbad(mtd, ofs);
		if (ret && !force) {
			printf("Skip erase bad block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			continue;
		}

		instr.addr = ofs;
		instr.len = blocksize;
		instr.priv = force;
		instr.mtd = mtd;
		ret = mtd_erase(mtd, &instr);
		if (ret) {
			printf("erase failed block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			continue;
		}
	}

	return 0;
}

static int onenand_block_test(u32 start, u32 size)
{
	struct onenand_chip *this = mtd->priv;
	struct erase_info instr = {
		.callback	= NULL,
		.priv		= 0,
	};

	int blocks;
	loff_t ofs;
	int blocksize = 1 << this->erase_shift;
	int start_block, end_block;
	size_t retlen;
	u_char *buf;
	u_char *verify_buf;
	int ret;

	buf = malloc(blocksize);
	if (!buf) {
		printf("Not enough malloc space available!\n");
		return -1;
	}

	verify_buf = malloc(blocksize);
	if (!verify_buf) {
		printf("Not enough malloc space available!\n");
		return -1;
	}

	start_block = start >> this->erase_shift;
	end_block = (start + size) >> this->erase_shift;

	/* Protect boot-loader from badblock testing */
	if (start_block < 2)
		start_block = 2;

	if (end_block > (mtd->size >> this->erase_shift))
		end_block = mtd->size >> this->erase_shift;

	blocks = start_block;
	ofs = start;
	while (blocks < end_block) {
		printf("\rTesting block %d at 0x%x", (u32)(ofs >> this->erase_shift), (u32)ofs);

		ret = mtd_block_isbad(mtd, ofs);
		if (ret) {
			printf("Skip erase bad block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			goto next;
		}

		instr.addr = ofs;
		instr.len = blocksize;
		ret = mtd_erase(mtd, &instr);
		if (ret) {
			printk("Erase failed 0x%x, %d\n", (u32)ofs, ret);
			goto next;
		}

		ret = mtd_write(mtd, ofs, blocksize, &retlen, buf);
		if (ret) {
			printk("Write failed 0x%x, %d\n", (u32)ofs, ret);
			goto next;
		}

		ret = mtd_read(mtd, ofs, blocksize, &retlen, verify_buf);
		if (ret) {
			printk("Read failed 0x%x, %d\n", (u32)ofs, ret);
			goto next;
		}

		if (memcmp(buf, verify_buf, blocksize))
			printk("\nRead/Write test failed at 0x%x\n", (u32)ofs);

next:
		ofs += blocksize;
		blocks++;
	}
	printf("...Done\n");

	free(buf);
	free(verify_buf);

	return 0;
}

static int onenand_dump(struct mtd_info *mtd, ulong off, int only_oob)
{
	int i;
	u_char *datbuf, *oobbuf, *p;
	struct mtd_oob_ops ops;
	loff_t addr;

	datbuf = malloc(mtd->writesize + mtd->oobsize);
	oobbuf = malloc(mtd->oobsize);
	if (!datbuf || !oobbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(mtd->writesize - 1);
	addr = (loff_t) off;
	memset(&ops, 0, sizeof(ops));
	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf;
	ops.len = mtd->writesize;
	ops.ooblen = mtd->oobsize;
	ops.retlen = 0;
	i = mtd_read_oob(mtd, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		free(datbuf);
		free(oobbuf);
		return 1;
	}
	printf("Page %08lx dump:\n", off);
	i = mtd->writesize >> 4;
	p = datbuf;

	while (i--) {
		if (!only_oob)
			printf("\t%02x %02x %02x %02x %02x %02x %02x %02x"
			       "  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			       p[8], p[9], p[10], p[11], p[12], p[13], p[14],
			       p[15]);
		p += 16;
	}
	puts("OOB:\n");
	i = mtd->oobsize >> 3;
	p = oobbuf;

	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
	free(datbuf);
	free(oobbuf);

	return 0;
}

static int do_onenand_info(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	printf("%s\n", mtd->name);
	return 0;
}

static int do_onenand_bad(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;

	mtd = &onenand_mtd;
	/* Currently only one OneNAND device is supported */
	printf("\nDevice %d bad blocks:\n", 0);
	for (ofs = 0; ofs < mtd->size; ofs += mtd->erasesize) {
		if (mtd_block_isbad(mtd, ofs))
			printf("  %08x\n", (u32)ofs);
	}

	return 0;
}

static int do_onenand_read(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	char *s;
	int oob = 0;
	ulong addr, ofs;
	size_t len;
	int ret = 0;
	size_t retlen = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	s = strchr(argv[0], '.');
	if ((s != NULL) && (!strcmp(s, ".oob")))
		oob = 1;

	addr = (ulong)simple_strtoul(argv[1], NULL, 16);

	printf("\nOneNAND read: ");
	if (arg_off_size_onenand(argc - 2, argv + 2, &ofs, &len) != 0)
		return 1;

	ret = onenand_block_read(ofs, len, &retlen, (u8 *)addr, oob);

	printf(" %d bytes read: %s\n", retlen, ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_onenand_write(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr, ofs;
	size_t len;
	int ret = 0, withoob = 0;
	size_t retlen = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	if (strncmp(argv[0] + 6, "yaffs", 5) == 0)
		withoob = 1;

	addr = (ulong)simple_strtoul(argv[1], NULL, 16);

	printf("\nOneNAND write: ");
	if (arg_off_size_onenand(argc - 2, argv + 2, &ofs, &len) != 0)
		return 1;

	ret = onenand_block_write(ofs, len, &retlen, (u8 *)addr, withoob);

	printf(" %d bytes written: %s\n", retlen, ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_onenand_erase(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;
	int ret = 0;
	size_t len;
	int force;

	/*
	 * Syntax is:
	 *   0       1     2       3    4
	 *   onenand erase [force] [off size]
	 */
	argc--;
	argv++;
	if (argc)
	{
		if (!strcmp("force", argv[0]))
		{
			force = 1;
			argc--;
			argv++;
		}
	}
	printf("\nOneNAND erase: ");

	/* skip first two or three arguments, look for offset and size */
	if (arg_off_size_onenand(argc, argv, &ofs, &len) != 0)
		return 1;

	ret = onenand_block_erase(ofs, len, force);

	printf("%s\n", ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_onenand_test(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;
	int ret = 0;
	size_t len;

	/*
	 * Syntax is:
	 *   0       1     2       3    4
	 *   onenand test [force] [off size]
	 */

	printf("\nOneNAND test: ");

	/* skip first two or three arguments, look for offset and size */
	if (arg_off_size_onenand(argc - 1, argv + 1, &ofs, &len) != 0)
		return 1;

	ret = onenand_block_test(ofs, len);

	printf("%s\n", ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_onenand_dump(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong ofs;
	int ret = 0;
	char *s;

	if (argc < 2)
		return CMD_RET_USAGE;

	s = strchr(argv[0], '.');
	ofs = (int)simple_strtoul(argv[1], NULL, 16);

	if (s != NULL && strcmp(s, ".oob") == 0)
		ret = onenand_dump(mtd, ofs, 1);
	else
		ret = onenand_dump(mtd, ofs, 0);

	return ret == 0 ? 1 : 0;
}

static int do_onenand_markbad(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	ulong addr;

	argc -= 2;
	argv += 2;

	if (argc <= 0)
		return CMD_RET_USAGE;

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

static cmd_tbl_t cmd_onenand_sub[] = {
	U_BOOT_CMD_MKENT(info, 1, 0, do_onenand_info, "", ""),
	U_BOOT_CMD_MKENT(bad, 1, 0, do_onenand_bad, "", ""),
	U_BOOT_CMD_MKENT(read, 4, 0, do_onenand_read, "", ""),
	U_BOOT_CMD_MKENT(write, 4, 0, do_onenand_write, "", ""),
	U_BOOT_CMD_MKENT(write.yaffs, 4, 0, do_onenand_write, "", ""),
	U_BOOT_CMD_MKENT(erase, 3, 0, do_onenand_erase, "", ""),
	U_BOOT_CMD_MKENT(test, 3, 0, do_onenand_test, "", ""),
	U_BOOT_CMD_MKENT(dump, 2, 0, do_onenand_dump, "", ""),
	U_BOOT_CMD_MKENT(markbad, CONFIG_SYS_MAXARGS, 0, do_onenand_markbad, "", ""),
};

#ifdef CONFIG_NEEDS_MANUAL_RELOC
void onenand_reloc(void) {
	fixup_cmdtable(cmd_onenand_sub, ARRAY_SIZE(cmd_onenand_sub));
}
#endif

static int do_onenand(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	mtd = &onenand_mtd;

	/* Strip off leading 'onenand' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_onenand_sub[0], ARRAY_SIZE(cmd_onenand_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	onenand,	CONFIG_SYS_MAXARGS,	1,	do_onenand,
	"OneNAND sub-system",
	"info - show available OneNAND devices\n"
	"onenand bad - show bad blocks\n"
	"onenand read[.oob] addr off size\n"
	"onenand write[.yaffs] addr off size\n"
	"    read/write 'size' bytes starting at offset 'off'\n"
	"    to/from memory address 'addr', skipping bad blocks.\n"
	"onenand erase [force] [off size] - erase 'size' bytes from\n"
	"onenand test [off size] - test 'size' bytes from\n"
	"    offset 'off' (entire device if not specified)\n"
	"onenand dump[.oob] off - dump page\n"
	"onenand markbad off [...] - mark bad block(s) at offset (UNSAFE)"
);
