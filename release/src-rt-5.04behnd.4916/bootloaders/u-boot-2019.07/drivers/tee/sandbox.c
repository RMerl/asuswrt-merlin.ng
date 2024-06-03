// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Linaro Limited
 */
#include <common.h>
#include <dm.h>
#include <sandboxtee.h>
#include <tee.h>
#include <tee/optee_ta_avb.h>

/*
 * The sandbox tee driver tries to emulate a generic Trusted Exectution
 * Environment (TEE) with the Trusted Application (TA) OPTEE_TA_AVB
 * available.
 */

static const u32 pstorage_max = 16;
/**
 * struct ta_entry - TA entries
 * @uuid:		UUID of an emulated TA
 * @open_session	Called when a session is openened to the TA
 * @invoke_func		Called when a function in the TA is to be invoked
 *
 * This struct is used to register TAs in this sandbox emulation of a TEE.
 */
struct ta_entry {
	struct tee_optee_ta_uuid uuid;
	u32 (*open_session)(struct udevice *dev, uint num_params,
			    struct tee_param *params);
	u32 (*invoke_func)(struct udevice *dev,
			   u32 func, uint num_params,
			   struct tee_param *params);
};

#ifdef CONFIG_OPTEE_TA_AVB
static u32 get_attr(uint n, uint num_params, struct tee_param *params)
{
	if (n >= num_params)
		return TEE_PARAM_ATTR_TYPE_NONE;

	return params[n].attr;
}

static u32 check_params(u8 p0, u8 p1, u8 p2, u8 p3, uint num_params,
			struct tee_param *params)
{
	u8 p[] = { p0, p1, p2, p3};
	uint n;

	for (n = 0; n < ARRAY_SIZE(p); n++)
		if (p[n] != get_attr(n, num_params, params))
			goto bad_params;

	for (; n < num_params; n++)
		if (get_attr(n, num_params, params))
			goto bad_params;

	return TEE_SUCCESS;

bad_params:
	printf("Bad param attrs\n");

	return TEE_ERROR_BAD_PARAMETERS;
}

static u32 ta_avb_open_session(struct udevice *dev, uint num_params,
			       struct tee_param *params)
{
	/*
	 * We don't expect additional parameters when opening a session to
	 * this TA.
	 */
	return check_params(TEE_PARAM_ATTR_TYPE_NONE, TEE_PARAM_ATTR_TYPE_NONE,
			    TEE_PARAM_ATTR_TYPE_NONE, TEE_PARAM_ATTR_TYPE_NONE,
			    num_params, params);
}

static u32 ta_avb_invoke_func(struct udevice *dev, u32 func, uint num_params,
			      struct tee_param *params)
{
	struct sandbox_tee_state *state = dev_get_priv(dev);
	ENTRY e, *ep;
	char *name;
	u32 res;
	uint slot;
	u64 val;
	char *value;
	u32 value_sz;

	switch (func) {
	case TA_AVB_CMD_READ_ROLLBACK_INDEX:
		res = check_params(TEE_PARAM_ATTR_TYPE_VALUE_INPUT,
				   TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   num_params, params);
		if (res)
			return res;

		slot = params[0].u.value.a;
		if (slot >= ARRAY_SIZE(state->ta_avb_rollback_indexes)) {
			printf("Rollback index slot out of bounds %u\n", slot);
			return TEE_ERROR_BAD_PARAMETERS;
		}

		val = state->ta_avb_rollback_indexes[slot];
		params[1].u.value.a = val >> 32;
		params[1].u.value.b = val;
		return TEE_SUCCESS;

	case TA_AVB_CMD_WRITE_ROLLBACK_INDEX:
		res = check_params(TEE_PARAM_ATTR_TYPE_VALUE_INPUT,
				   TEE_PARAM_ATTR_TYPE_VALUE_INPUT,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   num_params, params);
		if (res)
			return res;

		slot = params[0].u.value.a;
		if (slot >= ARRAY_SIZE(state->ta_avb_rollback_indexes)) {
			printf("Rollback index slot out of bounds %u\n", slot);
			return TEE_ERROR_BAD_PARAMETERS;
		}

		val = (u64)params[1].u.value.a << 32 | params[1].u.value.b;
		if (val < state->ta_avb_rollback_indexes[slot])
			return TEE_ERROR_SECURITY;

		state->ta_avb_rollback_indexes[slot] = val;
		return TEE_SUCCESS;

	case TA_AVB_CMD_READ_LOCK_STATE:
		res = check_params(TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   num_params, params);
		if (res)
			return res;

		params[0].u.value.a = state->ta_avb_lock_state;
		return TEE_SUCCESS;

	case TA_AVB_CMD_WRITE_LOCK_STATE:
		res = check_params(TEE_PARAM_ATTR_TYPE_VALUE_INPUT,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   num_params, params);
		if (res)
			return res;

		if (state->ta_avb_lock_state != params[0].u.value.a) {
			state->ta_avb_lock_state = params[0].u.value.a;
			memset(state->ta_avb_rollback_indexes, 0,
			       sizeof(state->ta_avb_rollback_indexes));
		}

		return TEE_SUCCESS;
	case TA_AVB_CMD_READ_PERSIST_VALUE:
		res = check_params(TEE_PARAM_ATTR_TYPE_MEMREF_INPUT,
				   TEE_PARAM_ATTR_TYPE_MEMREF_INOUT,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   num_params, params);
		if (res)
			return res;

		name = params[0].u.memref.shm->addr;

		value = params[1].u.memref.shm->addr;
		value_sz = params[1].u.memref.size;

		e.key = name;
		e.data = NULL;
		hsearch_r(e, FIND, &ep, &state->pstorage_htab, 0);
		if (!ep)
			return TEE_ERROR_ITEM_NOT_FOUND;

		value_sz = strlen(ep->data) + 1;
		memcpy(value, ep->data, value_sz);

		return TEE_SUCCESS;
	case TA_AVB_CMD_WRITE_PERSIST_VALUE:
		res = check_params(TEE_PARAM_ATTR_TYPE_MEMREF_INPUT,
				   TEE_PARAM_ATTR_TYPE_MEMREF_INPUT,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   TEE_PARAM_ATTR_TYPE_NONE,
				   num_params, params);
		if (res)
			return res;

		name = params[0].u.memref.shm->addr;

		value = params[1].u.memref.shm->addr;
		value_sz = params[1].u.memref.size;

		e.key = name;
		e.data = NULL;
		hsearch_r(e, FIND, &ep, &state->pstorage_htab, 0);
		if (ep)
			hdelete_r(e.key, &state->pstorage_htab, 0);

		e.key = name;
		e.data = value;
		hsearch_r(e, ENTER, &ep, &state->pstorage_htab, 0);
		if (!ep)
			return TEE_ERROR_OUT_OF_MEMORY;

		return TEE_SUCCESS;

	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
#endif /*OPTEE_TA_AVB*/

static const struct ta_entry ta_entries[] = {
#ifdef CONFIG_OPTEE_TA_AVB
	{ .uuid = TA_AVB_UUID,
	  .open_session = ta_avb_open_session,
	  .invoke_func = ta_avb_invoke_func,
	},
#endif
};

static void sandbox_tee_get_version(struct udevice *dev,
				    struct tee_version_data *vers)
{
	struct tee_version_data v = {
		.gen_caps = TEE_GEN_CAP_GP | TEE_GEN_CAP_REG_MEM,
	};

	*vers = v;
}

static int sandbox_tee_close_session(struct udevice *dev, u32 session)
{
	struct sandbox_tee_state *state = dev_get_priv(dev);

	if (!state->ta || state->session != session)
		return -EINVAL;

	state->session = 0;
	state->ta = NULL;

	return 0;
}

static const struct ta_entry *find_ta_entry(u8 uuid[TEE_UUID_LEN])
{
	struct tee_optee_ta_uuid u;
	uint n;

	tee_optee_ta_uuid_from_octets(&u, uuid);

	for (n = 0; n < ARRAY_SIZE(ta_entries); n++)
		if (!memcmp(&u, &ta_entries[n].uuid, sizeof(u)))
			return ta_entries + n;

	return NULL;
}

static int sandbox_tee_open_session(struct udevice *dev,
				    struct tee_open_session_arg *arg,
				    uint num_params, struct tee_param *params)
{
	struct sandbox_tee_state *state = dev_get_priv(dev);
	const struct ta_entry *ta;

	if (state->ta) {
		printf("A session is already open\n");
		return -EBUSY;
	}

	ta = find_ta_entry(arg->uuid);
	if (!ta) {
		printf("Cannot find TA\n");
		arg->ret = TEE_ERROR_ITEM_NOT_FOUND;
		arg->ret_origin = TEE_ORIGIN_TEE;

		return 0;
	}

	arg->ret = ta->open_session(dev, num_params, params);
	arg->ret_origin = TEE_ORIGIN_TRUSTED_APP;

	if (!arg->ret) {
		state->ta = (void *)ta;
		state->session = 1;
		arg->session = state->session;
	} else {
		printf("Cannot open session, TA returns error\n");
	}

	return 0;
}

static int sandbox_tee_invoke_func(struct udevice *dev,
				   struct tee_invoke_arg *arg,
				   uint num_params, struct tee_param *params)
{
	struct sandbox_tee_state *state = dev_get_priv(dev);
	struct ta_entry *ta = state->ta;

	if (!arg->session) {
		printf("Missing session\n");
		return -EINVAL;
	}

	if (!ta) {
		printf("TA session not available\n");
		return -EINVAL;
	}

	if (arg->session != state->session) {
		printf("Session mismatch\n");
		return -EINVAL;
	}

	arg->ret = ta->invoke_func(dev, arg->func, num_params, params);
	arg->ret_origin = TEE_ORIGIN_TRUSTED_APP;

	return 0;
}

static int sandbox_tee_shm_register(struct udevice *dev, struct tee_shm *shm)
{
	struct sandbox_tee_state *state = dev_get_priv(dev);

	state->num_shms++;

	return 0;
}

static int sandbox_tee_shm_unregister(struct udevice *dev, struct tee_shm *shm)
{
	struct sandbox_tee_state *state = dev_get_priv(dev);

	state->num_shms--;

	return 0;
}

static int sandbox_tee_remove(struct udevice *dev)
{
	struct sandbox_tee_state *state = dev_get_priv(dev);

	hdestroy_r(&state->pstorage_htab);

	return 0;
}

static int sandbox_tee_probe(struct udevice *dev)
{
	struct sandbox_tee_state *state = dev_get_priv(dev);
	/*
	 * With this hastable we emulate persistent storage,
	 * which should contain persistent values
	 * between different sessions/command invocations.
	 */
	if (!hcreate_r(pstorage_max, &state->pstorage_htab))
		return TEE_ERROR_OUT_OF_MEMORY;

	return 0;
}

static const struct tee_driver_ops sandbox_tee_ops = {
	.get_version = sandbox_tee_get_version,
	.open_session = sandbox_tee_open_session,
	.close_session = sandbox_tee_close_session,
	.invoke_func = sandbox_tee_invoke_func,
	.shm_register = sandbox_tee_shm_register,
	.shm_unregister = sandbox_tee_shm_unregister,
};

static const struct udevice_id sandbox_tee_match[] = {
	{ .compatible = "sandbox,tee" },
	{},
};

U_BOOT_DRIVER(sandbox_tee) = {
	.name = "sandbox_tee",
	.id = UCLASS_TEE,
	.of_match = sandbox_tee_match,
	.ops = &sandbox_tee_ops,
	.priv_auto_alloc_size = sizeof(struct sandbox_tee_state),
	.probe = sandbox_tee_probe,
	.remove = sandbox_tee_remove,
};
