// SPDX-License-Identifier: GPL-2.0+
/*
 * dfu_nand.c -- DFU for NAND routines.
 *
 * Copyright (C) 2012-2013 Texas Instruments, Inc.
 *
 * Based on dfu_mmc.c which is:
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <div64.h>
#include <dfu.h>
#include <linux/mtd/mtd.h>
#include <jffs2/load_kernel.h>
#include <nand.h>

static int nand_block_op(enum dfu_op op, struct dfu_entity *dfu,
			u64 offset, void *buf, long *len)
{
	loff_t start, lim;
	size_t count, actual;
	int ret;
	struct mtd_info *mtd;

	/* if buf == NULL return total size of the area */
	if (buf == NULL) {
		*len = dfu->data.nand.size;
		return 0;
	}

	start = dfu->data.nand.start + offset + dfu->bad_skip;
	lim = dfu->data.nand.start + dfu->data.nand.size - start;
	count = *len;

	mtd = get_nand_dev_by_index(nand_curr_device);

	if (nand_curr_device < 0 ||
	    nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE ||
	    !mtd) {
		printf("%s: invalid nand device\n", __func__);
		return -1;
	}

	if (op == DFU_OP_READ) {
		ret = nand_read_skip_bad(mtd, start, &count, &actual,
					 lim, buf);
	} else {
		nand_erase_options_t opts;

		memset(&opts, 0, sizeof(opts));
		opts.offset = start;
		opts.length = count;
		opts.spread = 1;
		opts.quiet = 1;
		opts.lim = lim;
		/* first erase */
		ret = nand_erase_opts(mtd, &opts);
		if (ret)
			return ret;
		/* then write */
		ret = nand_write_skip_bad(mtd, start, &count, &actual,
					  lim, buf, WITH_WR_VERIFY);
	}

	if (ret != 0) {
		printf("%s: nand_%s_skip_bad call failed at %llx!\n",
		       __func__, op == DFU_OP_READ ? "read" : "write",
		       start);
		return ret;
	}

	/*
	 * Find out where we stopped writing data.  This can be deeper into
	 * the NAND than we expected due to having to skip bad blocks.  So
	 * we must take this into account for the next write, if any.
	 */
	if (actual > count)
		dfu->bad_skip += actual - count;

	return ret;
}

static inline int nand_block_write(struct dfu_entity *dfu,
		u64 offset, void *buf, long *len)
{
	return nand_block_op(DFU_OP_WRITE, dfu, offset, buf, len);
}

static inline int nand_block_read(struct dfu_entity *dfu,
		u64 offset, void *buf, long *len)
{
	return nand_block_op(DFU_OP_READ, dfu, offset, buf, len);
}

static int dfu_write_medium_nand(struct dfu_entity *dfu,
		u64 offset, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = nand_block_write(dfu, offset, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

int dfu_get_medium_size_nand(struct dfu_entity *dfu, u64 *size)
{
	*size = dfu->data.nand.size;

	return 0;
}

static int dfu_read_medium_nand(struct dfu_entity *dfu, u64 offset, void *buf,
		long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = nand_block_read(dfu, offset, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

static int dfu_flush_medium_nand(struct dfu_entity *dfu)
{
	int ret = 0;
	u64 off;

	/* in case of ubi partition, erase rest of the partition */
	if (dfu->data.nand.ubi) {
		struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
		nand_erase_options_t opts;

		if (nand_curr_device < 0 ||
		    nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE ||
		    !mtd) {
			printf("%s: invalid nand device\n", __func__);
			return -1;
		}

		memset(&opts, 0, sizeof(opts));
		off = dfu->offset;
		if ((off & (mtd->erasesize - 1)) != 0) {
			/*
			 * last write ended with unaligned length
			 * sector is erased, jump to next
			 */
			off = off & ~((mtd->erasesize - 1));
			off += mtd->erasesize;
		}
		opts.offset = dfu->data.nand.start + off +
				dfu->bad_skip;
		opts.length = dfu->data.nand.start +
				dfu->data.nand.size - opts.offset;
		ret = nand_erase_opts(mtd, &opts);
		if (ret != 0)
			printf("Failure erase: %d\n", ret);
	}

	return ret;
}

unsigned int dfu_polltimeout_nand(struct dfu_entity *dfu)
{
	/*
	 * Currently, Poll Timeout != 0 is only needed on nand
	 * ubi partition, as the not used sectors need an erase
	 */
	if (dfu->data.nand.ubi)
		return DFU_MANIFEST_POLL_TIMEOUT;

	return DFU_DEFAULT_POLL_TIMEOUT;
}

int dfu_fill_entity_nand(struct dfu_entity *dfu, char *devstr, char *s)
{
	char *st;
	int ret, dev, part;

	dfu->data.nand.ubi = 0;
	dfu->dev_type = DFU_DEV_NAND;
	st = strsep(&s, " ");
	if (!strcmp(st, "raw")) {
		dfu->layout = DFU_RAW_ADDR;
		dfu->data.nand.start = simple_strtoul(s, &s, 16);
		s++;
		dfu->data.nand.size = simple_strtoul(s, &s, 16);
	} else if ((!strcmp(st, "part")) || (!strcmp(st, "partubi"))) {
		char mtd_id[32];
		struct mtd_device *mtd_dev;
		u8 part_num;
		struct part_info *pi;

		dfu->layout = DFU_RAW_ADDR;

		dev = simple_strtoul(s, &s, 10);
		s++;
		part = simple_strtoul(s, &s, 10);

		sprintf(mtd_id, "%s%d,%d", "nand", dev, part - 1);
		printf("using id '%s'\n", mtd_id);

		mtdparts_init();

		ret = find_dev_and_part(mtd_id, &mtd_dev, &part_num, &pi);
		if (ret != 0) {
			printf("Could not locate '%s'\n", mtd_id);
			return -1;
		}

		dfu->data.nand.start = pi->offset;
		dfu->data.nand.size = pi->size;
		if (!strcmp(st, "partubi"))
			dfu->data.nand.ubi = 1;
	} else {
		printf("%s: Memory layout (%s) not supported!\n", __func__, st);
		return -1;
	}

	dfu->get_medium_size = dfu_get_medium_size_nand;
	dfu->read_medium = dfu_read_medium_nand;
	dfu->write_medium = dfu_write_medium_nand;
	dfu->flush_medium = dfu_flush_medium_nand;
	dfu->poll_timeout = dfu_polltimeout_nand;

	/* initial state */
	dfu->inited = 0;

	return 0;
}
