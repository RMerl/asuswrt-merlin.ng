/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2018 Linaro Limited
 */

#ifndef __OPTEE_PRIVATE_H
#define __OPTEE_PRIVATE_H

#include <tee.h>
#include <log.h>

/**
 * struct optee_private - OP-TEE driver private data
 * @rpmb_mmc:		mmc device for the RPMB partition
 * @rpmb_dev_id:	mmc device id matching @rpmb_mmc
 * @rpmb_original_part:	the previosly active partition on the mmc device,
 *			used to restore active the partition when the RPMB
 *			accesses are finished
 */
struct optee_private {
	struct mmc *rpmb_mmc;
	int rpmb_dev_id;
	int rpmb_original_part;
};

struct optee_msg_arg;

void optee_suppl_cmd(struct udevice *dev, struct tee_shm *shm_arg,
		     void **page_list);

#ifdef CONFIG_SUPPORT_EMMC_RPMB
/**
 * optee_suppl_cmd_rpmb() - route RPMB frames to mmc
 * @dev:	device with the selected RPMB partition
 * @arg:	OP-TEE message holding the frames to transmit to the mmc
 *		and space for the response frames.
 *
 * Routes signed (MACed) RPMB frames from OP-TEE Secure OS to MMC and vice
 * versa to manipulate the RPMB partition.
 */
void optee_suppl_cmd_rpmb(struct udevice *dev, struct optee_msg_arg *arg);

/**
 * optee_suppl_rpmb_release() - release mmc device
 * @dev:	mmc device
 *
 * Releases the mmc device and restores the previously selected partition.
 */
void optee_suppl_rpmb_release(struct udevice *dev);
#else
static inline void optee_suppl_cmd_rpmb(struct udevice *dev,
					struct optee_msg_arg *arg)
{
	debug("OPTEE_MSG_RPC_CMD_RPMB not implemented\n");
	arg->ret = TEE_ERROR_NOT_IMPLEMENTED;
}

static inline void optee_suppl_rpmb_release(struct udevice *dev)
{
}
#endif

#ifdef CONFIG_DM_I2C
/**
 * optee_suppl_cmd_i2c_transfer() - route I2C requests to an I2C chip
 * @arg:	OP-TEE message (layout specified in optee_msg.h) defining the
 *		transfer mode (read/write), adapter, chip and control flags.
 *
 * Handles OP-TEE requests to transfer data to the I2C chip on the I2C adapter.
 */
void optee_suppl_cmd_i2c_transfer(struct optee_msg_arg *arg);
#else
static inline void optee_suppl_cmd_i2c_transfer(struct optee_msg_arg *arg)
{
	debug("OPTEE_MSG_RPC_CMD_I2C_TRANSFER not implemented\n");
	arg->ret = TEE_ERROR_NOT_IMPLEMENTED;
}
#endif

void *optee_alloc_and_init_page_list(void *buf, ulong len, u64 *phys_buf_ptr);

#endif /* __OPTEE_PRIVATE_H */
