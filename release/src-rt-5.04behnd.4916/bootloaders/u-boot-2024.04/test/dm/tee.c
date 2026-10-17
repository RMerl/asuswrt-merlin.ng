// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <dm/test.h>
#include <sandboxtee.h>
#include <tee.h>
#include <test/test.h>
#include <test/ut.h>
#include <tee/optee_ta_avb.h>
#include <tee/optee_ta_rpc_test.h>

static int open_session(struct udevice *dev, u32 *session,
			struct tee_optee_ta_uuid *uuid)
{
	struct tee_open_session_arg arg;
	int rc;

	memset(&arg, 0, sizeof(arg));
	tee_optee_ta_uuid_to_octets(arg.uuid, uuid);
	rc = tee_open_session(dev, &arg, 0, NULL);
	if (rc)
		return rc;
	if (arg.ret)
		return -EIO;
	*session = arg.session;

	return 0;
}

static int invoke_func_avb(struct udevice *dev, u32 session)
{
	struct tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT };
	struct tee_invoke_arg arg;

	memset(&arg, 0, sizeof(arg));
	arg.session = session;
	arg.func = TA_AVB_CMD_READ_LOCK_STATE;

	if (tee_invoke_func(dev, &arg, 1, &param) || arg.ret)
		return -1;

	return 0;
}

static int invoke_func_rpc_test(struct udevice *dev, u32 session,
				u64 op, u64 busnum, u64 chip_addr,
				u64 xfer_flags, u8 *buf, size_t buf_size)
{
	struct tee_param param[2];
	struct tee_invoke_arg arg;
	struct tee_shm *shm_buf;
	int rc;

	memset(&arg, 0, sizeof(arg));
	arg.session = session;
	arg.func = op;

	rc = tee_shm_alloc(dev, buf_size,
			   TEE_SHM_ALLOC, &shm_buf);
	if (rc)
		return rc;

	if (op == TA_RPC_TEST_CMD_I2C_WRITE)
		memcpy(shm_buf->addr, buf, buf_size);

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = busnum;
	param[0].u.value.b = chip_addr;
	param[0].u.value.c = xfer_flags;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = buf_size;

	if (tee_invoke_func(dev, &arg, 2, param) || arg.ret) {
		rc = -1;
		goto out;
	}

	if (op == TA_RPC_TEST_CMD_I2C_READ)
		memcpy(buf, shm_buf->addr, buf_size);
out:
	tee_shm_free(shm_buf);
	return rc;
}

static int match(struct tee_version_data *vers, const void *data)
{
	return vers->gen_caps & TEE_GEN_CAP_GP;
}

struct test_tee_vars {
	struct tee_shm *reg_shm;
	struct tee_shm *alloc_shm;
};

static int test_tee(struct unit_test_state *uts, struct test_tee_vars *vars)
{
	struct tee_version_data vers;
	struct udevice *dev;
	struct sandbox_tee_state *state;
	struct tee_optee_ta_uuid avb_uuid = TA_AVB_UUID;
	u32 session = 0;
	int rc;
	u8 data[128];

	dev = tee_find_device(NULL, match, NULL, &vers);
	ut_assert(dev);
	state = dev_get_priv(dev);
	ut_assert(!state->session);

	rc = open_session(dev, &session, &avb_uuid);
	ut_assert(!rc);
	ut_assert(session == state->session);

	rc = invoke_func_avb(dev, session);
	ut_assert(!rc);

	rc = tee_close_session(dev, session);
	ut_assert(!rc);
	ut_assert(!state->session);

	ut_assert(!state->num_shms);
	rc = tee_shm_register(dev, data, sizeof(data), 0, &vars->reg_shm);
	ut_assert(!rc);
	ut_assert(state->num_shms == 1);

	rc = tee_shm_alloc(dev, 256, 0, &vars->alloc_shm);
	ut_assert(!rc);
	ut_assert(state->num_shms == 2);

	ut_assert(tee_shm_is_registered(vars->reg_shm, dev));
	ut_assert(tee_shm_is_registered(vars->alloc_shm, dev));

	tee_shm_free(vars->reg_shm);
	vars->reg_shm = NULL;
	tee_shm_free(vars->alloc_shm);
	vars->alloc_shm = NULL;
	ut_assert(!state->num_shms);

	return rc;
}

#define I2C_BUF_SIZE 64

static int test_tee_rpc(struct unit_test_state *uts)
{
	struct tee_version_data vers;
	struct udevice *dev;
	struct sandbox_tee_state *state;
	struct tee_optee_ta_uuid rpc_test_uuid = TA_RPC_TEST_UUID;
	u32 session = 0;
	int rc;

	char *test_str = "Test string";
	u8 data[I2C_BUF_SIZE] = {0};
	u8 data_from_eeprom[I2C_BUF_SIZE] = {0};

	/* Use sandbox I2C EEPROM emulation; bus: 0, chip: 0x2c */
	u64 bus = 0;
	u64 chip = 0x2c;
	u64 xfer_flags = 0;

	dev = tee_find_device(NULL, match, NULL, &vers);
	ut_assert(dev);
	state = dev_get_priv(dev);
	ut_assert(!state->session);

	/* Test RPC call asking for I2C service */
	rc = open_session(dev, &session, &rpc_test_uuid);
	ut_assert(!rc);
	ut_assert(session == state->session);

	/* Write buffer */
	strncpy((char *)data, test_str, strlen(test_str));
	rc = invoke_func_rpc_test(dev, session, TA_RPC_TEST_CMD_I2C_WRITE,
				  bus, chip, xfer_flags, data, sizeof(data));
	ut_assert(!rc);

	/* Read buffer */
	rc = invoke_func_rpc_test(dev, session, TA_RPC_TEST_CMD_I2C_READ,
				  bus, chip, xfer_flags, data_from_eeprom,
				  sizeof(data_from_eeprom));
	ut_assert(!rc);

	/* Compare */
	ut_assert(!memcmp(data, data_from_eeprom, sizeof(data)));

	rc = tee_close_session(dev, session);
	ut_assert(!rc);
	ut_assert(!state->session);

	return rc;
}

static int dm_test_tee(struct unit_test_state *uts)
{
	struct test_tee_vars vars = { NULL, NULL };
	int rc = test_tee(uts, &vars);

	if (rc)
		goto out;

	if (IS_ENABLED(CONFIG_OPTEE_TA_RPC_TEST))
		rc = test_tee_rpc(uts);
out:
	/* In case test_tee() asserts these may still remain allocated */
	tee_shm_free(vars.reg_shm);
	tee_shm_free(vars.alloc_shm);

	return rc;
}

DM_TEST(dm_test_tee, UT_TESTF_SCAN_FDT);
