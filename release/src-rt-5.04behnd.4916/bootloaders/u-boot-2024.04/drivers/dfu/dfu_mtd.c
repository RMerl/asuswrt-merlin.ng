// SPDX-License-Identifier: GPL-2.0+
/*
 * dfu_mtd.c -- DFU for MTD device.
 *
 * Copyright (C) 2019,STMicroelectronics - All Rights Reserved
 *
 * Based on dfu_nand.c
 */

#include <common.h>
#include <dfu.h>
#include <mtd.h>
#include <linux/err.h>
#include <linux/ctype.h>

static bool mtd_is_aligned_with_block_size(struct mtd_info *mtd, u64 size)
{
	return !do_div(size, mtd->erasesize);
}

/* Logic taken from cmd/mtd.c:mtd_oob_write_is_empty() */
static bool mtd_page_is_empty(struct mtd_oob_ops *op)
{
	int i;

	for (i = 0; i < op->len; i++)
		if (op->datbuf[i] != 0xff)
			return false;

	/* oob is not used, with MTD_OPS_AUTO_OOB & ooblen=0 */

	return true;
}

static int mtd_block_op(enum dfu_op op, struct dfu_entity *dfu,
			u64 offset, void *buf, long *len)
{
	u64 off, lim, remaining, lock_ofs, lock_len;
	struct mtd_info *mtd = dfu->data.mtd.info;
	struct mtd_oob_ops io_op = {};
	int ret = 0;
	bool has_pages = mtd->type == MTD_NANDFLASH ||
			 mtd->type == MTD_MLCNANDFLASH;

	/* if buf == NULL return total size of the area */
	if (!buf) {
		*len = dfu->data.mtd.size;
		return 0;
	}

	off = lock_ofs = dfu->data.mtd.start + offset + dfu->bad_skip;
	lim = dfu->data.mtd.start + dfu->data.mtd.size;

	if (off >= lim) {
		printf("Limit reached 0x%llx\n", lim);
		*len = 0;
		return op == DFU_OP_READ ? 0 : -EIO;
	}
	/* limit request with the available size */
	if (off + *len >= lim)
		*len = lim - off;

	if (!mtd_is_aligned_with_block_size(mtd, off)) {
		printf("Offset not aligned with a block (0x%x)\n",
		       mtd->erasesize);
		return 0;
	}

	/* first erase */
	if (op == DFU_OP_WRITE) {
		struct erase_info erase_op = {};

		remaining = lock_len = round_up(*len, mtd->erasesize);
		erase_op.mtd = mtd;
		erase_op.addr = off;
		erase_op.len = mtd->erasesize;
		erase_op.scrub = 0;

		debug("Unlocking the mtd device\n");
		ret = mtd_unlock(mtd, lock_ofs, lock_len);
		if (ret && ret != -EOPNOTSUPP) {
			printf("MTD device unlock failed\n");
			return 0;
		}

		while (remaining) {
			if (erase_op.addr + remaining > lim) {
				printf("Limit reached 0x%llx while erasing at offset 0x%llx, remaining 0x%llx\n",
				       lim, erase_op.addr, remaining);
				return -EIO;
			}

			/* Skip the block if it is bad, don't erase it again */
			ret = mtd_block_isbad(mtd, erase_op.addr);
			if (ret) {
				printf("Skipping %s at 0x%08llx\n",
				       ret == 1 ? "bad block" : "bbt reserved",
				       erase_op.addr);
				erase_op.addr += mtd->erasesize;
				continue;
			}

			ret = mtd_erase(mtd, &erase_op);

			if (ret) {
				/* If this is not -EIO, we have no idea what to do. */
				if (ret == -EIO) {
					printf("Marking bad block at 0x%08llx (%d)\n",
					       erase_op.fail_addr, ret);
					ret = mtd_block_markbad(mtd, erase_op.addr);
				}
				/* Abort if it is not -EIO or can't mark bad */
				if (ret) {
					printf("Failure while erasing at offset 0x%llx (%d)\n",
					       erase_op.fail_addr, ret);
					return ret;
				}
			} else {
				remaining -= mtd->erasesize;
			}

			/* Continue erase behind the current block */
			erase_op.addr += mtd->erasesize;
		}
	}

	io_op.mode = MTD_OPS_AUTO_OOB;
	io_op.len = *len;
	if (has_pages && io_op.len > mtd->writesize)
		io_op.len = mtd->writesize;
	io_op.ooblen = 0;
	io_op.datbuf = buf;
	io_op.oobbuf = NULL;

	/* Loop over to do the actual read/write */
	remaining = *len;
	while (remaining) {
		if (off + remaining > lim) {
			printf("Limit reached 0x%llx while %s at offset 0x%llx\n",
			       lim, op == DFU_OP_READ ? "reading" : "writing",
			       off);
			if (op == DFU_OP_READ) {
				*len -= remaining;
				return 0;
			} else {
				return -EIO;
			}
		}

		/* Skip the block if it is bad */
		if (mtd_is_aligned_with_block_size(mtd, off) &&
		    mtd_block_isbad(mtd, off)) {
			off += mtd->erasesize;
			dfu->bad_skip += mtd->erasesize;
			continue;
		}

		if (op == DFU_OP_READ)
			ret = mtd_read_oob(mtd, off, &io_op);
		else if (has_pages && dfu->data.mtd.ubi && mtd_page_is_empty(&io_op)) {
			/* in case of ubi partition, do not write an empty page, only skip it */
			ret = 0;
			io_op.retlen = mtd->writesize;
			io_op.oobretlen = mtd->oobsize;
		} else {
			ret = mtd_write_oob(mtd, off, &io_op);
		}

		if (ret) {
			printf("Failure while %s at offset 0x%llx\n",
			       op == DFU_OP_READ ? "reading" : "writing", off);
			return -EIO;
		}

		off += io_op.retlen;
		remaining -= io_op.retlen;
		io_op.datbuf += io_op.retlen;
		io_op.len = remaining;
		if (has_pages && io_op.len > mtd->writesize)
			io_op.len = mtd->writesize;
	}

	if (op == DFU_OP_WRITE) {
		/* Write done, lock again */
		debug("Locking the mtd device\n");
		ret = mtd_lock(mtd, lock_ofs, lock_len);
		if (ret == -EOPNOTSUPP)
			ret = 0;
		else if (ret)
			printf("MTD device lock failed\n");
	}
	return ret;
}

static int dfu_get_medium_size_mtd(struct dfu_entity *dfu, u64 *size)
{
	*size = dfu->data.mtd.info->size;

	return 0;
}

static int dfu_read_medium_mtd(struct dfu_entity *dfu, u64 offset, void *buf,
			       long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mtd_block_op(DFU_OP_READ, dfu, offset, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

static int dfu_write_medium_mtd(struct dfu_entity *dfu,
				u64 offset, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mtd_block_op(DFU_OP_WRITE, dfu, offset, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

static int dfu_flush_medium_mtd(struct dfu_entity *dfu)
{
	struct mtd_info *mtd = dfu->data.mtd.info;
	u64 remaining;
	int ret;

	/* in case of ubi partition, erase rest of the partition */
	if (dfu->data.mtd.ubi) {
		struct erase_info erase_op = {};

		erase_op.mtd = dfu->data.mtd.info;
		erase_op.addr = round_up(dfu->data.mtd.start + dfu->offset +
					 dfu->bad_skip, mtd->erasesize);
		erase_op.len = mtd->erasesize;
		erase_op.scrub = 0;

		remaining = dfu->data.mtd.start + dfu->data.mtd.size -
			    erase_op.addr;

		while (remaining) {
			ret = mtd_erase(mtd, &erase_op);

			if (ret) {
				/* Abort if its not a bad block error */
				if (ret != -EIO)
					break;
				printf("Skipping bad block at 0x%08llx\n",
				       erase_op.addr);
			}

			/* Skip bad block and continue behind it */
			erase_op.addr += mtd->erasesize;
			remaining -= mtd->erasesize;
		}
	}
	return 0;
}

static unsigned int dfu_polltimeout_mtd(struct dfu_entity *dfu)
{
	/*
	 * Currently, Poll Timeout != 0 is only needed on nand
	 * ubi partition, as sectors which are not used need
	 * to be erased
	 */
	if (dfu->data.mtd.ubi)
		return DFU_MANIFEST_POLL_TIMEOUT;

	return DFU_DEFAULT_POLL_TIMEOUT;
}

int dfu_fill_entity_mtd(struct dfu_entity *dfu, char *devstr, char **argv, int argc)
{
	char *s;
	struct mtd_info *mtd;
	int part;

	mtd = get_mtd_device_nm(devstr);
	if (IS_ERR_OR_NULL(mtd))
		return -ENODEV;
	put_mtd_device(mtd);

	dfu->dev_type = DFU_DEV_MTD;
	dfu->data.mtd.info = mtd;
	dfu->max_buf_size = mtd->erasesize;
	if (argc < 1)
		return -EINVAL;

	if (!strcmp(argv[0], "raw")) {
		if (argc != 3)
			return -EINVAL;
		dfu->layout = DFU_RAW_ADDR;
		dfu->data.mtd.start = hextoul(argv[1], &s);
		if (*s)
			return -EINVAL;
		dfu->data.mtd.size = hextoul(argv[2], &s);
		if (*s)
			return -EINVAL;
	} else if ((!strcmp(argv[0], "part")) || (!strcmp(argv[0], "partubi"))) {
		struct mtd_info *partition;
		int partnum = 0;
		bool part_found = false;

		if (argc != 2)
			return -EINVAL;

		dfu->layout = DFU_RAW_ADDR;

		part = dectoul(argv[1], &s);
		if (*s)
			return -EINVAL;

		/* register partitions with MTDIDS/MTDPARTS or OF fallback */
		mtd_probe_devices();

		partnum = 0;
		list_for_each_entry(partition, &mtd->partitions, node) {
			partnum++;
			if (partnum == part) {
				part_found = true;
				break;
			}
		}
		if (!part_found) {
			printf("No partition %d in %s\n", part, mtd->name);
			return -1;
		}
		log_debug("partition %d:%s in %s\n", partnum, partition->name, mtd->name);

		dfu->data.mtd.start = partition->offset;
		dfu->data.mtd.size = partition->size;
		if (!strcmp(argv[0], "partubi"))
			dfu->data.mtd.ubi = 1;
	} else {
		printf("%s: Memory layout (%s) not supported!\n", __func__, argv[0]);
		return -1;
	}

	if (!mtd_is_aligned_with_block_size(mtd, dfu->data.mtd.start)) {
		printf("Offset not aligned with a block (0x%x)\n",
		       mtd->erasesize);
		return -EINVAL;
	}
	if (!mtd_is_aligned_with_block_size(mtd, dfu->data.mtd.size)) {
		printf("Size not aligned with a block (0x%x)\n",
		       mtd->erasesize);
		return -EINVAL;
	}

	dfu->get_medium_size = dfu_get_medium_size_mtd;
	dfu->read_medium = dfu_read_medium_mtd;
	dfu->write_medium = dfu_write_medium_mtd;
	dfu->flush_medium = dfu_flush_medium_mtd;
	dfu->poll_timeout = dfu_polltimeout_mtd;

	/* initial state */
	dfu->inited = 0;

	return 0;
}
