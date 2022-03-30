// SPDX-License-Identifier: GPL-2.0+
/*
 * dfu.c -- DFU back-end routines
 *
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <div64.h>
#include <dfu.h>
#include <ext4fs.h>
#include <fat.h>
#include <mmc.h>

static unsigned char *dfu_file_buf;
static u64 dfu_file_buf_len;
static long dfu_file_buf_filled;

static int mmc_block_op(enum dfu_op op, struct dfu_entity *dfu,
			u64 offset, void *buf, long *len)
{
	struct mmc *mmc;
	u32 blk_start, blk_count, n = 0;
	int ret, part_num_bkp = 0;

	mmc = find_mmc_device(dfu->data.mmc.dev_num);
	if (!mmc) {
		pr_err("Device MMC %d - not found!", dfu->data.mmc.dev_num);
		return -ENODEV;
	}

	/*
	 * We must ensure that we work in lba_blk_size chunks, so ALIGN
	 * this value.
	 */
	*len = ALIGN(*len, dfu->data.mmc.lba_blk_size);

	blk_start = dfu->data.mmc.lba_start +
			(u32)lldiv(offset, dfu->data.mmc.lba_blk_size);
	blk_count = *len / dfu->data.mmc.lba_blk_size;
	if (blk_start + blk_count >
			dfu->data.mmc.lba_start + dfu->data.mmc.lba_size) {
		puts("Request would exceed designated area!\n");
		return -EINVAL;
	}

	if (dfu->data.mmc.hw_partition >= 0) {
		part_num_bkp = mmc_get_blk_desc(mmc)->hwpart;
		ret = blk_select_hwpart_devnum(IF_TYPE_MMC,
					       dfu->data.mmc.dev_num,
					       dfu->data.mmc.hw_partition);
		if (ret)
			return ret;
	}

	debug("%s: %s dev: %d start: %d cnt: %d buf: 0x%p\n", __func__,
	      op == DFU_OP_READ ? "MMC READ" : "MMC WRITE",
	      dfu->data.mmc.dev_num, blk_start, blk_count, buf);
	switch (op) {
	case DFU_OP_READ:
		n = blk_dread(mmc_get_blk_desc(mmc), blk_start, blk_count, buf);
		break;
	case DFU_OP_WRITE:
		n = blk_dwrite(mmc_get_blk_desc(mmc), blk_start, blk_count,
			       buf);
		break;
	default:
		pr_err("Operation not supported\n");
	}

	if (n != blk_count) {
		pr_err("MMC operation failed");
		if (dfu->data.mmc.hw_partition >= 0)
			blk_select_hwpart_devnum(IF_TYPE_MMC,
						 dfu->data.mmc.dev_num,
						 part_num_bkp);
		return -EIO;
	}

	if (dfu->data.mmc.hw_partition >= 0) {
		ret = blk_select_hwpart_devnum(IF_TYPE_MMC,
					       dfu->data.mmc.dev_num,
					       part_num_bkp);
		if (ret)
			return ret;
	}

	return 0;
}

static int mmc_file_buffer(struct dfu_entity *dfu, void *buf, long *len)
{
	if (dfu_file_buf_len + *len > CONFIG_SYS_DFU_MAX_FILE_SIZE) {
		dfu_file_buf_len = 0;
		return -EINVAL;
	}

	/* Add to the current buffer. */
	memcpy(dfu_file_buf + dfu_file_buf_len, buf, *len);
	dfu_file_buf_len += *len;

	return 0;
}

static int mmc_file_op(enum dfu_op op, struct dfu_entity *dfu,
			void *buf, u64 *len)
{
	char dev_part_str[8];
	int ret;
	int fstype;
	loff_t size = 0;

	switch (dfu->layout) {
	case DFU_FS_FAT:
		fstype = FS_TYPE_FAT;
		break;
	case DFU_FS_EXT4:
		fstype = FS_TYPE_EXT;
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
		return -1;
	}

	snprintf(dev_part_str, sizeof(dev_part_str), "%d:%d",
		 dfu->data.mmc.dev, dfu->data.mmc.part);

	ret = fs_set_blk_dev("mmc", dev_part_str, fstype);
	if (ret) {
		puts("dfu: fs_set_blk_dev error!\n");
		return ret;
	}

	switch (op) {
	case DFU_OP_READ:
		ret = fs_read(dfu->name, (size_t)buf, 0, 0, &size);
		if (ret) {
			puts("dfu: fs_read error!\n");
			return ret;
		}
		*len = size;
		break;
	case DFU_OP_WRITE:
		ret = fs_write(dfu->name, (size_t)buf, 0, *len, &size);
		if (ret) {
			puts("dfu: fs_write error!\n");
			return ret;
		}
		break;
	case DFU_OP_SIZE:
		ret = fs_size(dfu->name, &size);
		if (ret) {
			puts("dfu: fs_size error!\n");
			return ret;
		}
		*len = size;
		break;
	default:
		return -1;
	}

	return ret;
}

int dfu_write_medium_mmc(struct dfu_entity *dfu,
		u64 offset, void *buf, long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mmc_block_op(DFU_OP_WRITE, dfu, offset, buf, len);
		break;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = mmc_file_buffer(dfu, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

int dfu_flush_medium_mmc(struct dfu_entity *dfu)
{
	int ret = 0;

	if (dfu->layout != DFU_RAW_ADDR) {
		/* Do stuff here. */
		ret = mmc_file_op(DFU_OP_WRITE, dfu, dfu_file_buf,
				&dfu_file_buf_len);

		/* Now that we're done */
		dfu_file_buf_len = 0;
	}

	return ret;
}

int dfu_get_medium_size_mmc(struct dfu_entity *dfu, u64 *size)
{
	int ret;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		*size = dfu->data.mmc.lba_size * dfu->data.mmc.lba_blk_size;
		return 0;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		dfu_file_buf_filled = -1;
		ret = mmc_file_op(DFU_OP_SIZE, dfu, NULL, size);
		if (ret < 0)
			return ret;
		if (*size > CONFIG_SYS_DFU_MAX_FILE_SIZE)
			return -1;
		return 0;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
		return -1;
	}
}

static int mmc_file_unbuffer(struct dfu_entity *dfu, u64 offset, void *buf,
			     long *len)
{
	int ret;
	u64 file_len;

	if (dfu_file_buf_filled == -1) {
		ret = mmc_file_op(DFU_OP_READ, dfu, dfu_file_buf, &file_len);
		if (ret < 0)
			return ret;
		dfu_file_buf_filled = file_len;
	}
	if (offset + *len > dfu_file_buf_filled)
		return -EINVAL;

	/* Add to the current buffer. */
	memcpy(buf, dfu_file_buf + offset, *len);

	return 0;
}

int dfu_read_medium_mmc(struct dfu_entity *dfu, u64 offset, void *buf,
		long *len)
{
	int ret = -1;

	switch (dfu->layout) {
	case DFU_RAW_ADDR:
		ret = mmc_block_op(DFU_OP_READ, dfu, offset, buf, len);
		break;
	case DFU_FS_FAT:
	case DFU_FS_EXT4:
		ret = mmc_file_unbuffer(dfu, offset, buf, len);
		break;
	default:
		printf("%s: Layout (%s) not (yet) supported!\n", __func__,
		       dfu_get_layout(dfu->layout));
	}

	return ret;
}

void dfu_free_entity_mmc(struct dfu_entity *dfu)
{
	if (dfu_file_buf) {
		free(dfu_file_buf);
		dfu_file_buf = NULL;
	}
}

/*
 * @param s Parameter string containing space-separated arguments:
 *	1st:
 *		raw	(raw read/write)
 *		fat	(files)
 *		ext4	(^)
 *		part	(partition image)
 *	2nd and 3rd:
 *		lba_start and lba_size, for raw write
 *		mmc_dev and mmc_part, for filesystems and part
 *	4th (optional):
 *		mmcpart <num> (access to HW eMMC partitions)
 */
int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *devstr, char *s)
{
	const char *entity_type;
	size_t second_arg;
	size_t third_arg;

	struct mmc *mmc;

	const char *argv[3];
	const char **parg = argv;

	dfu->data.mmc.dev_num = simple_strtoul(devstr, NULL, 10);

	for (; parg < argv + sizeof(argv) / sizeof(*argv); ++parg) {
		*parg = strsep(&s, " ");
		if (*parg == NULL) {
			pr_err("Invalid number of arguments.\n");
			return -ENODEV;
		}
	}

	entity_type = argv[0];
	/*
	 * Base 0 means we'll accept (prefixed with 0x or 0) base 16, 8,
	 * with default 10.
	 */
	second_arg = simple_strtoul(argv[1], NULL, 0);
	third_arg = simple_strtoul(argv[2], NULL, 0);

	mmc = find_mmc_device(dfu->data.mmc.dev_num);
	if (mmc == NULL) {
		pr_err("Couldn't find MMC device no. %d.\n",
		      dfu->data.mmc.dev_num);
		return -ENODEV;
	}

	if (mmc_init(mmc)) {
		pr_err("Couldn't init MMC device.\n");
		return -ENODEV;
	}

	dfu->data.mmc.hw_partition = -EINVAL;
	if (!strcmp(entity_type, "raw")) {
		dfu->layout			= DFU_RAW_ADDR;
		dfu->data.mmc.lba_start		= second_arg;
		dfu->data.mmc.lba_size		= third_arg;
		dfu->data.mmc.lba_blk_size	= mmc->read_bl_len;

		/*
		 * Check for an extra entry at dfu_alt_info env variable
		 * specifying the mmc HW defined partition number
		 */
		if (s)
			if (!strcmp(strsep(&s, " "), "mmcpart"))
				dfu->data.mmc.hw_partition =
					simple_strtoul(s, NULL, 0);

	} else if (!strcmp(entity_type, "part")) {
		disk_partition_t partinfo;
		struct blk_desc *blk_dev = mmc_get_blk_desc(mmc);
		int mmcdev = second_arg;
		int mmcpart = third_arg;

		if (part_get_info(blk_dev, mmcpart, &partinfo) != 0) {
			pr_err("Couldn't find part #%d on mmc device #%d\n",
			      mmcpart, mmcdev);
			return -ENODEV;
		}

		dfu->layout			= DFU_RAW_ADDR;
		dfu->data.mmc.lba_start		= partinfo.start;
		dfu->data.mmc.lba_size		= partinfo.size;
		dfu->data.mmc.lba_blk_size	= partinfo.blksz;
	} else if (!strcmp(entity_type, "fat")) {
		dfu->layout = DFU_FS_FAT;
	} else if (!strcmp(entity_type, "ext4")) {
		dfu->layout = DFU_FS_EXT4;
	} else {
		pr_err("Memory layout (%s) not supported!\n", entity_type);
		return -ENODEV;
	}

	/* if it's NOT a raw write */
	if (strcmp(entity_type, "raw")) {
		dfu->data.mmc.dev = second_arg;
		dfu->data.mmc.part = third_arg;
	}

	dfu->dev_type = DFU_DEV_MMC;
	dfu->get_medium_size = dfu_get_medium_size_mmc;
	dfu->read_medium = dfu_read_medium_mmc;
	dfu->write_medium = dfu_write_medium_mmc;
	dfu->flush_medium = dfu_flush_medium_mmc;
	dfu->inited = 0;
	dfu->free_entity = dfu_free_entity_mmc;

	/* Check if file buffer is ready */
	if (!dfu_file_buf) {
		dfu_file_buf = memalign(CONFIG_SYS_CACHELINE_SIZE,
					CONFIG_SYS_DFU_MAX_FILE_SIZE);
		if (!dfu_file_buf) {
			pr_err("Could not memalign 0x%x bytes",
			      CONFIG_SYS_DFU_MAX_FILE_SIZE);
			return -ENOMEM;
		}
	}

	return 0;
}
