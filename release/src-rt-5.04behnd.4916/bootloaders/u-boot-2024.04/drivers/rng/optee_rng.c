// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 */
#define LOG_CATEGORY UCLASS_RNG

#include <common.h>

#include <rng.h>
#include <tee.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/sizes.h>
#include <tee/optee_service.h>

#define DRIVER_NAME	"optee-rng"

#define TEE_ERROR_HEALTH_TEST_FAIL	0x00000001

/*
 * TA_CMD_GET_ENTROPY - Get Entropy from RNG
 *
 * param[0] (inout memref) - Entropy buffer memory reference
 * param[1] unused
 * param[2] unused
 * param[3] unused
 *
 * Result:
 * TEE_SUCCESS - Invoke command success
 * TEE_ERROR_BAD_PARAMETERS - Incorrect input param
 * TEE_ERROR_NOT_SUPPORTED - Requested entropy size greater than size of pool
 * TEE_ERROR_HEALTH_TEST_FAIL - Continuous health testing failed
 */
#define TA_CMD_GET_ENTROPY		0x0

#define MAX_ENTROPY_REQ_SZ		SZ_4K

#define TA_HWRNG_UUID { 0xab7a617c, 0xb8e7, 0x4d8f, \
			{ 0x83, 0x01, 0xd0, 0x9b, 0x61, 0x03, 0x6b, 0x64 } }

OPTEE_SERVICE_DRIVER(optee_rng, TA_HWRNG_UUID, DRIVER_NAME);

/** open_session_ta_hwrng() - Open session with hwrng Trusted App
 *
 * @dev:		device
 * @session_id:		return the RNG TA session identifier
 * Return:		0 if ok
 */
static int open_session_ta_hwrng(struct udevice *dev, u32 *session_id)
{
	const struct tee_optee_ta_uuid uuid = TA_HWRNG_UUID;
	struct tee_open_session_arg sess_arg = {0};
	int ret;

	/* Open session with hwrng Trusted App */
	tee_optee_ta_uuid_to_octets(sess_arg.uuid, &uuid);
	sess_arg.clnt_login = TEE_LOGIN_PUBLIC;

	ret = tee_open_session(dev->parent, &sess_arg, 0, NULL);
	if (ret || sess_arg.ret) {
		if (!ret)
			ret = -EIO;
		return ret;
	}

	*session_id = sess_arg.session;
	return 0;
}

/**
 * get_optee_rng_data() - read RNG data from OP-TEE TA
 *
 * @dev:		device
 * @session_id:		the RNG TA session identifier
 * @entropy_shm_pool:	shared memory pool used for TEE message
 * @buf:		buffer to receive data
 * @size:		size of buffer, limited by entropy_shm_pool size
 * Return:		0 if ok
 */
static int get_optee_rng_data(struct udevice *dev, u32 session_id,
			      struct tee_shm *entropy_shm_pool,
			      void *buf, size_t *size)
{
	int ret = 0;
	struct tee_invoke_arg arg = {0};
	struct tee_param param = {0};

	/* Invoke TA_CMD_GET_ENTROPY function of Trusted App */
	arg.func = TA_CMD_GET_ENTROPY;
	arg.session = session_id;

	/* Fill invoke cmd params */
	param.attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param.u.memref.shm = entropy_shm_pool;
	param.u.memref.size = *size;

	ret = tee_invoke_func(dev->parent, &arg, 1, &param);
	if (ret || arg.ret) {
		if (!ret)
			ret = -EPROTO;
		dev_err(dev, "TA_CMD_GET_ENTROPY invoke err: %d 0x%x\n", ret, arg.ret);
		*size = 0;

		return ret;
	}

	memcpy(buf, param.u.memref.shm->addr, param.u.memref.size);
	*size = param.u.memref.size;

	return 0;
}

/**
 * optee_rng_read() - rng read ops for OP-TEE RNG device
 *
 * @dev:		device
 * @buf:		buffer to receive data
 * @len:		size of buffer
 * Return:		0 if ok
 */
static int optee_rng_read(struct udevice *dev, void *buf, size_t len)
{
	size_t read = 0, rng_size = 0;
	struct tee_shm *entropy_shm_pool;
	u8 *data = buf;
	int ret;
	u32 session_id = 0;

	ret = open_session_ta_hwrng(dev, &session_id);
	if (ret) {
		dev_err(dev, "can't open session: %d\n", ret);
		return ret;
	}

	ret = tee_shm_alloc(dev->parent, MAX_ENTROPY_REQ_SZ, 0, &entropy_shm_pool);
	if (ret) {
		dev_err(dev, "tee_shm_alloc failed: %d\n", ret);
		goto session_close;
	}

	while (read < len) {
		rng_size = min(len - read, (size_t)MAX_ENTROPY_REQ_SZ);
		ret = get_optee_rng_data(dev, session_id, entropy_shm_pool, data, &rng_size);
		if (ret)
			goto shm_free;
		data += rng_size;
		read += rng_size;
	}

shm_free:
	tee_shm_free(entropy_shm_pool);

session_close:
	tee_close_session(dev->parent, session_id);

	return ret;
}

/**
 * optee_rng_probe() - probe function for OP-TEE RNG device
 *
 * @dev:		device
 * Return:		0 if ok
 */
static int optee_rng_probe(struct udevice *dev)
{
	int ret;
	u32 session_id;

	ret = open_session_ta_hwrng(dev, &session_id);
	if (ret) {
		dev_err(dev, "can't open session: %d\n", ret);
		return ret;
	}
	tee_close_session(dev->parent, session_id);

	return 0;
}

static const struct dm_rng_ops optee_rng_ops = {
	.read = optee_rng_read,
};

U_BOOT_DRIVER(optee_rng) = {
	.name = DRIVER_NAME,
	.id = UCLASS_RNG,
	.ops = &optee_rng_ops,
	.probe = optee_rng_probe,
};
