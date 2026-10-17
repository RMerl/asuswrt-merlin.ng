// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Eugeniu Rosca <rosca.eugeniu@gmail.com>
 *
 * Android Bootloader Control Block Header
 */

#ifndef __BCB_H__
#define __BCB_H__

#include <part.h>

enum bcb_field {
	BCB_FIELD_COMMAND,
	BCB_FIELD_STATUS,
	BCB_FIELD_RECOVERY,
	BCB_FIELD_STAGE
};

#if IS_ENABLED(CONFIG_CMD_BCB)

int bcb_find_partition_and_load(const char *iface,
				int devnum, char *partp);
int bcb_load(struct blk_desc *block_description,
	     struct disk_partition *disk_partition);
int bcb_set(enum bcb_field field, const char *value);

/**
 * bcb_get() - get the field value.
 * @field: field to get
 * @value_out: buffer to copy bcb field value to
 * @value_size: buffer size to avoid overflow in case
 *              value_out is smaller then the field value
 */
int bcb_get(enum bcb_field field, char *value_out, size_t value_size);

int bcb_store(void);
void bcb_reset(void);

#else

#include <linux/errno.h>

static inline int bcb_load(struct blk_desc *block_description,
			   struct disk_partition *disk_partition)
{
	return -EOPNOTSUPP;
}

static inline int bcb_find_partition_and_load(const char *iface,
					      int devnum, char *partp)
{
	return -EOPNOTSUPP;
}

static inline int bcb_set(enum bcb_field field, const char *value)
{
	return -EOPNOTSUPP;
}

static inline int bcb_get(enum bcb_field field, char *value_out)
{
	return -EOPNOTSUPP;
}

static inline int bcb_store(void)
{
	return -EOPNOTSUPP;
}

static inline void bcb_reset(void)
{
}
#endif

#endif /* __BCB_H__ */
