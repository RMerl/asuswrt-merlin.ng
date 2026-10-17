// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2018, Linaro Limited
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <tee.h>
#include <linux/types.h>

#include "optee_msg.h"
#include "optee_msg_supplicant.h"
#include "optee_private.h"
#include "optee_smc.h"

static void cmd_shm_alloc(struct udevice *dev, struct optee_msg_arg *arg,
			  void **page_list)
{
	int rc;
	struct tee_shm *shm;
	void *pl;
	u64 ph_ptr;

	arg->ret_origin = TEE_ORIGIN_COMMS;

	if (arg->num_params != 1 ||
	    arg->params[0].attr != OPTEE_MSG_ATTR_TYPE_VALUE_INPUT) {
		arg->ret = TEE_ERROR_BAD_PARAMETERS;
		return;
	}

	rc = __tee_shm_add(dev, 0, NULL, arg->params[0].u.value.b,
			   TEE_SHM_REGISTER | TEE_SHM_ALLOC, &shm);
	if (rc) {
		if (rc == -ENOMEM)
			arg->ret = TEE_ERROR_OUT_OF_MEMORY;
		else
			arg->ret = TEE_ERROR_GENERIC;
		return;
	}

	pl = optee_alloc_and_init_page_list(shm->addr, shm->size, &ph_ptr);
	if (!pl) {
		arg->ret = TEE_ERROR_OUT_OF_MEMORY;
		tee_shm_free(shm);
		return;
	}

	*page_list = pl;
	arg->params[0].attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT |
			      OPTEE_MSG_ATTR_NONCONTIG;
	arg->params[0].u.tmem.buf_ptr = ph_ptr;
	arg->params[0].u.tmem.size = shm->size;
	arg->params[0].u.tmem.shm_ref = (ulong)shm;
	arg->ret = TEE_SUCCESS;
}

static void cmd_shm_free(struct optee_msg_arg *arg)
{
	arg->ret_origin = TEE_ORIGIN_COMMS;

	if (arg->num_params != 1 ||
	    arg->params[0].attr != OPTEE_MSG_ATTR_TYPE_VALUE_INPUT) {
		arg->ret = TEE_ERROR_BAD_PARAMETERS;
		return;
	}

	tee_shm_free((struct tee_shm *)(ulong)arg->params[0].u.value.b);
	arg->ret = TEE_SUCCESS;
}

void optee_suppl_cmd(struct udevice *dev, struct tee_shm *shm_arg,
		     void **page_list)
{
	struct optee_msg_arg *arg = shm_arg->addr;

	switch (arg->cmd) {
	case OPTEE_MSG_RPC_CMD_SHM_ALLOC:
		cmd_shm_alloc(dev, arg, page_list);
		break;
	case OPTEE_MSG_RPC_CMD_SHM_FREE:
		cmd_shm_free(arg);
		break;
	case OPTEE_MSG_RPC_CMD_FS:
		debug("REE FS storage isn't available\n");
		arg->ret = TEE_ERROR_STORAGE_NOT_AVAILABLE;
		break;
	case OPTEE_MSG_RPC_CMD_RPMB:
		optee_suppl_cmd_rpmb(dev, arg);
		break;
	case OPTEE_MSG_RPC_CMD_I2C_TRANSFER:
		optee_suppl_cmd_i2c_transfer(arg);
		break;
	default:
		arg->ret = TEE_ERROR_NOT_IMPLEMENTED;
	}

	arg->ret_origin = TEE_ORIGIN_COMMS;
}
