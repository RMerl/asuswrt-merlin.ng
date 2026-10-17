// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Microsoft Corporation
 *
 * Authors:
 * Thirupathaiah Annapureddy <thiruan@microsoft.com>
 *
 * Description:
 * Device Driver for a firmware TPM as described here:
 * https://www.microsoft.com/en-us/research/publication/ftpm-software-implementation-tpm-chip/
 *
 * A reference implementation is available here:
 * https://github.com/microsoft/ms-tpm-20-ref/tree/master/Samples/ARM32-FirmwareTPM/optee_ta/fTPM
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <tpm-v2.h>
#include <tee.h>
#include <tee/optee_service.h>

#include "tpm_tis.h"
#include "tpm2_ftpm_tee.h"

OPTEE_SERVICE_DRIVER(optee_ftpm, TA_FTPM_UUID, "ftpm_tee");

/**
 * ftpm_tee_transceive() - send fTPM commands and retrieve fTPM response.
 * @sendbuf - address of the data to send, byte by byte
 * @send_size - length of the data to send
 * @recvbuf - address where to read the response, byte by byte.
 * @recv_len - pointer to the size of buffer
 *
 * Return:
 *	In case of success, returns 0.
 *	On failure, -errno
 */
static int ftpm_tee_transceive(struct udevice *dev, const u8 *sendbuf,
				size_t send_size, u8 *recvbuf,
				size_t *recv_len)
{
	struct ftpm_tee_private *context = dev_get_priv(dev);
	int rc = 0;
	size_t resp_len;
	u8 *resp_buf;
	struct tpm_output_header *resp_header;
	struct tee_invoke_arg transceive_args;
	struct tee_param command_params[4];
	struct tee_shm *shm;

	if (send_size > MAX_COMMAND_SIZE) {
		debug("%s:send_size=%zd exceeds MAX_COMMAND_SIZE\n",
			__func__, send_size);
		return -EIO;
	}

	shm = context->shm;
	memset(&transceive_args, 0, sizeof(transceive_args));
	memset(command_params, 0, sizeof(command_params));

	/* Invoke FTPM_OPTEE_TA_SUBMIT_COMMAND function of fTPM TA */
	transceive_args = (struct tee_invoke_arg) {
		.func = FTPM_OPTEE_TA_SUBMIT_COMMAND,
		.session = context->session,
	};

	/* Fill FTPM_OPTEE_TA_SUBMIT_COMMAND parameters */
	/* request */
	command_params[0] = (struct tee_param) {
		.attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT,
		.u.memref = {
			.shm = shm,
			.size = send_size,
			.shm_offs = 0,
		},
	};
	memset(command_params[0].u.memref.shm->addr, 0,
		(MAX_COMMAND_SIZE + MAX_RESPONSE_SIZE));
	memcpy(command_params[0].u.memref.shm->addr, sendbuf, send_size);

	/* response */
	command_params[1] = (struct tee_param) {
		.attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT,
		.u.memref = {
			.shm = shm,
			.size = MAX_RESPONSE_SIZE,
			.shm_offs = MAX_COMMAND_SIZE,
		},
	};

	rc = tee_invoke_func(context->tee_dev, &transceive_args, 4,
				command_params);
	if ((rc < 0) || (transceive_args.ret != 0)) {
		debug("%s:SUBMIT_COMMAND invoke error: 0x%x\n",
			__func__, transceive_args.ret);
		return (rc < 0) ? rc : transceive_args.ret;
	}

	resp_buf = command_params[1].u.memref.shm->addr +
		command_params[1].u.memref.shm_offs;
	resp_header = (struct tpm_output_header *)resp_buf;
	resp_len = be32_to_cpu(resp_header->length);

	/* sanity check resp_len*/
	if (resp_len < TPM_HEADER_SIZE) {
		debug("%s:tpm response header too small\n", __func__);
		return -EIO;
	}
	if (resp_len > MAX_RESPONSE_SIZE) {
		debug("%s:resp_len=%zd exceeds MAX_RESPONSE_SIZE\n",
			__func__, resp_len);
		return -EIO;
	}
	if (resp_len > *recv_len) {
		debug("%s:response length is bigger than receive buffer\n",
			__func__);
		return -EIO;
	}

	/* sanity checks look good, copy the response */
	memcpy(recvbuf,  resp_buf,  resp_len);
	*recv_len  = resp_len;

	return 0;
}

static int ftpm_tee_open(struct udevice *dev)
{
	struct ftpm_tee_private *context = dev_get_priv(dev);

	if (context->is_open)
		return -EBUSY;

	context->is_open = 1;

	return 0;
}

static int ftpm_tee_close(struct udevice *dev)
{
	struct ftpm_tee_private *context = dev_get_priv(dev);

	if (context->is_open)
		context->is_open = 0;

	return 0;
}

static int ftpm_tee_desc(struct udevice *dev, char *buf, int size)
{
	if (size < 32)
		return -ENOSPC;

	return snprintf(buf, size, "Microsoft OP-TEE fTPM");
}

static int ftpm_tee_match(struct tee_version_data *vers, const void *data)
{
	debug("%s:vers->gen_caps =0x%x\n", __func__, vers->gen_caps);

	/*
	 * Currently this driver only support GP Complaint OPTEE based fTPM TA
	 */
	return vers->gen_caps & TEE_GEN_CAP_GP;
}

static int ftpm_tee_probe(struct udevice *dev)
{
	int rc;
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	struct ftpm_tee_private *context = dev_get_priv(dev);
	struct tee_open_session_arg sess_arg;
	const struct tee_optee_ta_uuid uuid = TA_FTPM_UUID;

	memset(context, 0, sizeof(*context));

	/* Use the TPM v2 stack */
	priv->version = TPM_V2;
	priv->pcr_count = 24;
	priv->pcr_select_min = 3;

	/* Find TEE device */
	context->tee_dev = tee_find_device(NULL, ftpm_tee_match, NULL, NULL);
	if (!context->tee_dev) {
		debug("%s:tee_find_device failed\n", __func__);
		return -ENODEV;
	}

	/* Open a session with the fTPM TA */
	memset(&sess_arg, 0, sizeof(sess_arg));
	sess_arg.clnt_login = TEE_LOGIN_REE_KERNEL;
	tee_optee_ta_uuid_to_octets(sess_arg.uuid, &uuid);

	rc = tee_open_session(context->tee_dev, &sess_arg, 0, NULL);
	if ((rc < 0) || (sess_arg.ret != 0)) {
		debug("%s:tee_open_session failed, err=%x\n",
			__func__, sess_arg.ret);
		return -EIO;
	}
	context->session = sess_arg.session;

	/* Allocate dynamic shared memory with fTPM TA */
	rc = tee_shm_alloc(context->tee_dev,
			MAX_COMMAND_SIZE + MAX_RESPONSE_SIZE,
			0, &context->shm);
	if (rc) {
		debug("%s:tee_shm_alloc failed with rc = %d\n", __func__, rc);
		goto out_shm_alloc;
	}

	return 0;

out_shm_alloc:
	tee_close_session(context->tee_dev, context->session);

	return rc;
}

static int ftpm_tee_remove(struct udevice *dev)
{
	struct ftpm_tee_private *context = dev_get_priv(dev);
	int rc;

	/* tee_pre_remove frees any leftover TEE shared memory */

	/* close the existing session with fTPM TA*/
	rc = tee_close_session(context->tee_dev, context->session);
	debug("%s: tee_close_session - rc =%d\n", __func__, rc);

	return 0;
}

static const struct tpm_ops ftpm_tee_ops = {
	.open		= ftpm_tee_open,
	.close		= ftpm_tee_close,
	.get_desc	= ftpm_tee_desc,
	.xfer		= ftpm_tee_transceive,
};

static const struct udevice_id ftpm_tee_ids[] = {
	{ .compatible = "microsoft,ftpm" },
	{ }
};

U_BOOT_DRIVER(ftpm_tee) = {
	.name   = "ftpm_tee",
	.id     = UCLASS_TPM,
	.of_match = ftpm_tee_ids,
	.ops    = &ftpm_tee_ops,
	.probe	= ftpm_tee_probe,
	.remove	= ftpm_tee_remove,
	.flags	= DM_FLAG_OS_PREPARE,
	.priv_auto	= sizeof(struct ftpm_tee_private),
};
