/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2008,2010 Freescale Semiconductor, Inc
 * Copyright 2020 NXP
 * Andy Fleming
 *
 * Based (loosely) on the Linux code
 */

#ifndef _MMC_PRIVATE_H_
#define _MMC_PRIVATE_H_

#include <mmc.h>

int mmc_send_status(struct mmc *mmc, unsigned int *status);
int mmc_poll_for_busy(struct mmc *mmc, int timeout);

int mmc_set_blocklen(struct mmc *mmc, int len);

#if CONFIG_IS_ENABLED(BLK)
ulong mmc_bread(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		void *dst);
#else
ulong mmc_bread(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt,
		void *dst);
#endif

#if CONFIG_IS_ENABLED(MMC_WRITE)

#if CONFIG_IS_ENABLED(BLK)
ulong mmc_bwrite(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		 const void *src);
ulong mmc_berase(struct udevice *dev, lbaint_t start, lbaint_t blkcnt);
#else
ulong mmc_bwrite(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt,
		 const void *src);
ulong mmc_berase(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt);
#endif

#else /* CONFIG_SPL_MMC_WRITE is not defined */

/* declare dummies to reduce code size. */

#if CONFIG_IS_ENABLED(BLK)
static inline unsigned long mmc_berase(struct udevice *dev,
				       lbaint_t start, lbaint_t blkcnt)
{
	return 0;
}

static inline ulong mmc_bwrite(struct udevice *dev, lbaint_t start,
			       lbaint_t blkcnt, const void *src)
{
	return 0;
}
#else
static inline unsigned long mmc_berase(struct blk_desc *block_dev,
				       lbaint_t start, lbaint_t blkcnt)
{
	return 0;
}

static inline ulong mmc_bwrite(struct blk_desc *block_dev, lbaint_t start,
			       lbaint_t blkcnt, const void *src)
{
	return 0;
}
#endif

#endif /* CONFIG_SPL_BUILD */

#ifdef CONFIG_MMC_TRACE
void mmmc_trace_before_send(struct mmc *mmc, struct mmc_cmd *cmd);
void mmmc_trace_after_send(struct mmc *mmc, struct mmc_cmd *cmd, int ret);
void mmc_trace_state(struct mmc *mmc, struct mmc_cmd *cmd);
#else
static inline void mmmc_trace_before_send(struct mmc *mmc, struct mmc_cmd *cmd)
{
}

static inline void mmmc_trace_after_send(struct mmc *mmc, struct mmc_cmd *cmd,
					 int ret)
{
}

static inline void mmc_trace_state(struct mmc *mmc, struct mmc_cmd *cmd)
{
}
#endif

/**
 * mmc_get_next_devnum() - Get the next available MMC device number
 *
 * Return: next available device number (0 = first), or -ve on error
 */
int mmc_get_next_devnum(void);

/**
 * mmc_do_preinit() - Get an MMC device ready for use
 */
void mmc_do_preinit(void);

/**
 * mmc_list_init() - Set up the list of MMC devices
 */
void mmc_list_init(void);

/**
 * mmc_list_add() - Add a new MMC device to the list of devices
 *
 * @mmc:	Device to add
 */
void mmc_list_add(struct mmc *mmc);

/**
 * mmc_switch_part() - Switch to a new MMC hardware partition
 *
 * @mmc:	MMC device
 * @part_num:	Hardware partition number
 * Return: 0 if OK, -ve on error
 */
int mmc_switch_part(struct mmc *mmc, unsigned int part_num);

/**
 * mmc_switch() - Issue and MMC switch mode command
 *
 * @mmc:	MMC device
 * @set:	Unused
 * @index:	Cmdarg index
 * @value:	Cmdarg value
 * Return: 0 if OK, -ve on error
 */
int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value);

#endif /* _MMC_PRIVATE_H_ */
