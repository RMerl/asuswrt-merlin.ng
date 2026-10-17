// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2020 Foundries.io Ltd
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <tee.h>
#include "optee_msg.h"
#include "optee_private.h"

static int check_xfer_flags(struct udevice *chip, uint tee_flags)
{
	uint flags;
	int ret;

	ret = i2c_get_chip_flags(chip, &flags);
	if (ret)
		return ret;

	if (tee_flags & OPTEE_MSG_RPC_CMD_I2C_FLAGS_TEN_BIT) {
		if (!(flags & DM_I2C_CHIP_10BIT))
			return -EINVAL;
	} else {
		if (flags & DM_I2C_CHIP_10BIT)
			return -EINVAL;
	}

	return 0;
}

void optee_suppl_cmd_i2c_transfer(struct optee_msg_arg *arg)
{
	const u8 attr[] = {
		OPTEE_MSG_ATTR_TYPE_VALUE_INPUT,
		OPTEE_MSG_ATTR_TYPE_VALUE_INPUT,
		OPTEE_MSG_ATTR_TYPE_RMEM_INOUT,
		OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT,
	};
	struct udevice *chip_dev;
	struct tee_shm *shm;
	u8 *buf;
	int ret;

	if (arg->num_params != ARRAY_SIZE(attr) ||
	    arg->params[0].attr != attr[0] ||
	    arg->params[1].attr != attr[1] ||
	    arg->params[2].attr != attr[2] ||
	    arg->params[3].attr != attr[3]) {
		goto bad;
	}

	shm = (struct tee_shm *)(unsigned long)arg->params[2].u.rmem.shm_ref;
	buf = shm->addr;
	if (!buf)
		goto bad;

	if (i2c_get_chip_for_busnum((int)arg->params[0].u.value.b,
				    (int)arg->params[0].u.value.c,
				    0, &chip_dev))
		goto bad;

	if (check_xfer_flags(chip_dev, arg->params[1].u.value.a))
		goto bad;

	switch (arg->params[0].u.value.a) {
	case OPTEE_MSG_RPC_CMD_I2C_TRANSFER_RD:
		ret = dm_i2c_read(chip_dev, 0, buf,
				  (size_t)arg->params[2].u.rmem.size);
		break;
	case OPTEE_MSG_RPC_CMD_I2C_TRANSFER_WR:
		ret = dm_i2c_write(chip_dev, 0, buf,
				   (size_t)arg->params[2].u.rmem.size);
		break;
	default:
		goto bad;
	}

	if (ret) {
		arg->ret = TEE_ERROR_COMMUNICATION;
	} else {
		arg->params[3].u.value.a = arg->params[2].u.rmem.size;
		arg->ret = TEE_SUCCESS;
	}

	return;
bad:
	arg->ret = TEE_ERROR_BAD_PARAMETERS;
}
