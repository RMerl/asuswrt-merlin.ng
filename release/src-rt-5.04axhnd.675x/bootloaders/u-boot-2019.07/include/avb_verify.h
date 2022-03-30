
/*
 * (C) Copyright 2018, Linaro Limited
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	_AVB_VERIFY_H
#define _AVB_VERIFY_H

#include <../lib/libavb/libavb.h>
#include <mapmem.h>
#include <mmc.h>

#define AVB_MAX_ARGS			1024
#define VERITY_TABLE_OPT_RESTART	"restart_on_corruption"
#define VERITY_TABLE_OPT_LOGGING	"ignore_corruption"
#define ALLOWED_BUF_ALIGN		8

enum avb_boot_state {
	AVB_GREEN,
	AVB_YELLOW,
	AVB_ORANGE,
	AVB_RED,
};

struct AvbOpsData {
	struct AvbOps ops;
	int mmc_dev;
	enum avb_boot_state boot_state;
#ifdef CONFIG_OPTEE_TA_AVB
	struct udevice *tee;
	u32 session;
#endif
};

struct mmc_part {
	int dev_num;
	struct mmc *mmc;
	struct blk_desc *mmc_blk;
	disk_partition_t info;
};

enum mmc_io_type {
	IO_READ,
	IO_WRITE
};

AvbOps *avb_ops_alloc(int boot_device);
void avb_ops_free(AvbOps *ops);

char *avb_set_state(AvbOps *ops, enum avb_boot_state boot_state);
char *avb_set_enforce_verity(const char *cmdline);
char *avb_set_ignore_corruption(const char *cmdline);

char *append_cmd_line(char *cmdline_orig, char *cmdline_new);

/**
 * ============================================================================
 * I/O helper inline functions
 * ============================================================================
 */
static inline uint64_t calc_offset(struct mmc_part *part, int64_t offset)
{
	u64 part_size = part->info.size * part->info.blksz;

	if (offset < 0)
		return part_size + offset;

	return offset;
}

static inline size_t get_sector_buf_size(void)
{
	return (size_t)CONFIG_FASTBOOT_BUF_SIZE;
}

static inline void *get_sector_buf(void)
{
	return map_sysmem(CONFIG_FASTBOOT_BUF_ADDR, CONFIG_FASTBOOT_BUF_SIZE);
}

static inline bool is_buf_unaligned(void *buffer)
{
	return (bool)((uintptr_t)buffer % ALLOWED_BUF_ALIGN);
}

static inline int get_boot_device(AvbOps *ops)
{
	struct AvbOpsData *data;

	if (ops) {
		data = ops->user_data;
		if (data)
			return data->mmc_dev;
	}

	return -1;
}

#endif /* _AVB_VERIFY_H */
