// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018-2020 Linaro Limited
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <log.h>
#include <malloc.h>
#include <tee.h>
#include <linux/arm-smccc.h>
#include <linux/err.h>
#include <linux/io.h>
#include <tee/optee_service.h>

#include "optee_smc.h"
#include "optee_msg.h"
#include "optee_private.h"

#define PAGELIST_ENTRIES_PER_PAGE \
	((OPTEE_MSG_NONCONTIG_PAGE_SIZE / sizeof(u64)) - 1)

/*
 * PTA_DEVICE_ENUM interface exposed by OP-TEE to discover enumerated services
 */
#define PTA_DEVICE_ENUM		{ 0x7011a688, 0xddde, 0x4053, \
				  { 0xa5, 0xa9, 0x7b, 0x3c, 0x4d, 0xdf, 0x13, 0xb8 } }
/*
 * PTA_CMD_GET_DEVICES - List services without supplicant dependencies
 *
 * [out]    memref[0]: List of the UUIDs of service enumerated by OP-TEE
 */
#define PTA_CMD_GET_DEVICES		0x0

/*
 * PTA_CMD_GET_DEVICES_SUPP - List services depending on tee supplicant
 *
 * [out]    memref[0]: List of the UUIDs of service enumerated by OP-TEE
 */
#define PTA_CMD_GET_DEVICES_SUPP	0x1

typedef void (optee_invoke_fn)(unsigned long, unsigned long, unsigned long,
			       unsigned long, unsigned long, unsigned long,
			       unsigned long, unsigned long,
			       struct arm_smccc_res *);

struct optee_pdata {
	optee_invoke_fn *invoke_fn;
};

struct rpc_param {
	u32	a0;
	u32	a1;
	u32	a2;
	u32	a3;
	u32	a4;
	u32	a5;
	u32	a6;
	u32	a7;
};

static struct optee_service *find_service_driver(const struct tee_optee_ta_uuid *uuid)
{
	struct optee_service *service;
	u8 loc_uuid[TEE_UUID_LEN];
	size_t service_cnt, idx;

	service_cnt = ll_entry_count(struct optee_service, optee_service);
	service = ll_entry_start(struct optee_service, optee_service);

	for (idx = 0; idx < service_cnt; idx++, service++) {
		tee_optee_ta_uuid_to_octets(loc_uuid, &service->uuid);
		if (!memcmp(uuid, loc_uuid, sizeof(*uuid)))
			return service;
	}

	return NULL;
}

static int bind_service_list(struct udevice *dev, struct tee_shm *service_list, size_t count)
{
	const struct tee_optee_ta_uuid *service_uuid = (const void *)service_list->addr;
	struct optee_service *service;
	size_t idx;
	int ret;

	for (idx = 0; idx < count; idx++) {
		service = find_service_driver(service_uuid + idx);
		if (!service)
			continue;

		ret = device_bind_driver_to_node(dev, service->driver_name, service->driver_name,
						 dev_ofnode(dev), NULL);
		if (ret) {
			dev_warn(dev, "%s was not bound: %d, ignored\n", service->driver_name, ret);
			continue;
		}
	}

	return 0;
}

static int __enum_services(struct udevice *dev, struct tee_shm *shm, size_t *shm_size, u32 tee_sess,
			   unsigned int pta_cmd)
{
	struct tee_invoke_arg arg = { };
	struct tee_param param = { };
	int ret = 0;

	arg.func = pta_cmd;
	arg.session = tee_sess;

	/* Fill invoke cmd params */
	param.attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	param.u.memref.shm = shm;
	param.u.memref.size = *shm_size;

	ret = tee_invoke_func(dev, &arg, 1, &param);
	if (ret || (arg.ret && arg.ret != TEE_ERROR_SHORT_BUFFER)) {
		dev_err(dev, "Enumeration command 0x%x failed: 0x%x\n", pta_cmd, arg.ret);
		return -EINVAL;
	}

	*shm_size = param.u.memref.size;

	return 0;
}

static int enum_services(struct udevice *dev, struct tee_shm **shm, size_t *count, u32 tee_sess,
			 unsigned int pta_cmd)
{
	size_t shm_size = 0;
	int ret;

	ret = __enum_services(dev, NULL, &shm_size, tee_sess, pta_cmd);
	if (ret)
		return ret;

	if (!shm_size) {
		*count = 0;
		return 0;
	}

	ret = tee_shm_alloc(dev, shm_size, 0, shm);
	if (ret) {
		dev_err(dev, "Failed to allocated shared memory: %d\n", ret);
		return ret;
	}

	ret = __enum_services(dev, *shm, &shm_size, tee_sess, pta_cmd);
	if (!ret)
		*count = shm_size / sizeof(struct tee_optee_ta_uuid);

	return ret;
}

static int open_enum_session(struct udevice *dev, u32 *tee_sess)
{
	const struct tee_optee_ta_uuid pta_uuid = PTA_DEVICE_ENUM;
	struct tee_open_session_arg arg = { };
	int ret;

	tee_optee_ta_uuid_to_octets(arg.uuid, &pta_uuid);

	ret = tee_open_session(dev, &arg, 0, NULL);
	if (ret || arg.ret) {
		if (!ret)
			ret = -EIO;
		return ret;
	}

	*tee_sess = arg.session;

	return 0;
}

static int bind_service_drivers(struct udevice *dev)
{
	struct tee_shm *service_list = NULL;
	size_t service_count;
	u32 tee_sess;
	int ret, ret2;

	ret = open_enum_session(dev, &tee_sess);
	if (ret)
		return ret;

	ret = enum_services(dev, &service_list, &service_count, tee_sess,
			    PTA_CMD_GET_DEVICES);
	if (!ret && service_count)
		ret = bind_service_list(dev, service_list, service_count);

	tee_shm_free(service_list);
	service_list = NULL;

	ret2 = enum_services(dev, &service_list, &service_count, tee_sess,
			     PTA_CMD_GET_DEVICES_SUPP);
	if (!ret2 && service_count)
		ret2 = bind_service_list(dev, service_list, service_count);

	tee_shm_free(service_list);

	tee_close_session(dev, tee_sess);

	if (ret)
		return ret;

	return ret2;
}

/**
 * reg_pair_to_ptr() - Make a pointer of 2 32-bit values
 * @reg0:	High bits of the pointer
 * @reg1:	Low bits of the pointer
 *
 * Returns the combined result, note that if a pointer is 32-bit wide @reg0
 * will be discarded.
 */
static void *reg_pair_to_ptr(u32 reg0, u32 reg1)
{
	return (void *)(ulong)(((u64)reg0 << 32) | reg1);
}

/**
 * reg_pair_from_64() - Split a 64-bit value into two 32-bit values
 * @reg0:	High bits of @val
 * @reg1:	Low bits of @val
 * @val:	The value to split
 */
static void reg_pair_from_64(u32 *reg0, u32 *reg1, u64 val)
{
	*reg0 = val >> 32;
	*reg1 = val;
}

/**
 * optee_alloc_and_init_page_list() - Provide page list of memory buffer
 * @buf:		Start of buffer
 * @len:		Length of buffer
 * @phys_buf_ptr	Physical pointer with coded offset to page list
 *
 * Secure world doesn't share mapping with Normal world (U-Boot in this case)
 * so physical pointers are needed when sharing pointers.
 *
 * Returns a pointer page list on success or NULL on failure
 */
void *optee_alloc_and_init_page_list(void *buf, ulong len, u64 *phys_buf_ptr)
{
	const unsigned int page_size = OPTEE_MSG_NONCONTIG_PAGE_SIZE;
	const phys_addr_t page_mask = page_size - 1;
	u8 *buf_base;
	unsigned int page_offset;
	unsigned int num_pages;
	unsigned int list_size;
	unsigned int n;
	void *page_list;
	struct {
		u64 pages_list[PAGELIST_ENTRIES_PER_PAGE];
		u64 next_page_data;
	} *pages_data;

	/*
	 * A Memory buffer is described in chunks of 4k. The list of
	 * physical addresses has to be represented by a physical pointer
	 * too and a single list has to start at a 4k page and fit into
	 * that page. In order to be able to describe large memory buffers
	 * these 4k pages carrying physical addresses are linked together
	 * in a list. See OPTEE_MSG_ATTR_NONCONTIG in
	 * drivers/tee/optee/optee_msg.h for more information.
	 */

	page_offset = (ulong)buf & page_mask;
	num_pages = roundup(page_offset + len, page_size) / page_size;
	list_size = DIV_ROUND_UP(num_pages, PAGELIST_ENTRIES_PER_PAGE) *
		    page_size;
	page_list = memalign(page_size, list_size);
	if (!page_list)
		return NULL;

	pages_data = page_list;
	buf_base = (u8 *)rounddown((ulong)buf, page_size);
	n = 0;
	while (num_pages) {
		pages_data->pages_list[n] = virt_to_phys(buf_base);
		n++;
		buf_base += page_size;
		num_pages--;

		if (n == PAGELIST_ENTRIES_PER_PAGE) {
			pages_data->next_page_data =
				virt_to_phys(pages_data + 1);
			pages_data++;
			n = 0;
		}
	}

	*phys_buf_ptr = virt_to_phys(page_list) | page_offset;
	return page_list;
}

static void optee_get_version(struct udevice *dev,
			      struct tee_version_data *vers)
{
	struct tee_version_data v = {
		.gen_caps = TEE_GEN_CAP_GP | TEE_GEN_CAP_REG_MEM,
	};

	*vers = v;
}

static int get_msg_arg(struct udevice *dev, uint num_params,
		       struct tee_shm **shmp, struct optee_msg_arg **msg_arg)
{
	int rc;
	struct optee_msg_arg *ma;

	rc = __tee_shm_add(dev, OPTEE_MSG_NONCONTIG_PAGE_SIZE, NULL,
			   OPTEE_MSG_GET_ARG_SIZE(num_params), TEE_SHM_ALLOC,
			   shmp);
	if (rc)
		return rc;

	ma = (*shmp)->addr;
	memset(ma, 0, OPTEE_MSG_GET_ARG_SIZE(num_params));
	ma->num_params = num_params;
	*msg_arg = ma;

	return 0;
}

static int to_msg_param(struct optee_msg_param *msg_params, uint num_params,
			const struct tee_param *params)
{
	uint n;

	for (n = 0; n < num_params; n++) {
		const struct tee_param *p = params + n;
		struct optee_msg_param *mp = msg_params + n;

		switch (p->attr) {
		case TEE_PARAM_ATTR_TYPE_NONE:
			mp->attr = OPTEE_MSG_ATTR_TYPE_NONE;
			memset(&mp->u, 0, sizeof(mp->u));
			break;
		case TEE_PARAM_ATTR_TYPE_VALUE_INPUT:
		case TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT:
		case TEE_PARAM_ATTR_TYPE_VALUE_INOUT:
			mp->attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT + p->attr -
				   TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
			mp->u.value.a = p->u.value.a;
			mp->u.value.b = p->u.value.b;
			mp->u.value.c = p->u.value.c;
			break;
		case TEE_PARAM_ATTR_TYPE_MEMREF_INPUT:
		case TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT:
		case TEE_PARAM_ATTR_TYPE_MEMREF_INOUT:
			mp->attr = OPTEE_MSG_ATTR_TYPE_RMEM_INPUT + p->attr -
				   TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
			mp->u.rmem.shm_ref = (ulong)p->u.memref.shm;
			mp->u.rmem.size = p->u.memref.size;
			mp->u.rmem.offs = p->u.memref.shm_offs;
			break;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

static int from_msg_param(struct tee_param *params, uint num_params,
			  const struct optee_msg_param *msg_params)
{
	uint n;
	struct tee_shm *shm;

	for (n = 0; n < num_params; n++) {
		struct tee_param *p = params + n;
		const struct optee_msg_param *mp = msg_params + n;
		u32 attr = mp->attr & OPTEE_MSG_ATTR_TYPE_MASK;

		switch (attr) {
		case OPTEE_MSG_ATTR_TYPE_NONE:
			p->attr = TEE_PARAM_ATTR_TYPE_NONE;
			memset(&p->u, 0, sizeof(p->u));
			break;
		case OPTEE_MSG_ATTR_TYPE_VALUE_INPUT:
		case OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT:
		case OPTEE_MSG_ATTR_TYPE_VALUE_INOUT:
			p->attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT + attr -
				  OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
			p->u.value.a = mp->u.value.a;
			p->u.value.b = mp->u.value.b;
			p->u.value.c = mp->u.value.c;
			break;
		case OPTEE_MSG_ATTR_TYPE_RMEM_INPUT:
		case OPTEE_MSG_ATTR_TYPE_RMEM_OUTPUT:
		case OPTEE_MSG_ATTR_TYPE_RMEM_INOUT:
			p->attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT + attr -
				  OPTEE_MSG_ATTR_TYPE_RMEM_INPUT;
			p->u.memref.size = mp->u.rmem.size;
			shm = (struct tee_shm *)(ulong)mp->u.rmem.shm_ref;

			if (!shm) {
				p->u.memref.shm_offs = 0;
				p->u.memref.shm = NULL;
				break;
			}
			p->u.memref.shm_offs = mp->u.rmem.offs;
			p->u.memref.shm = shm;
			break;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

static void handle_rpc(struct udevice *dev, struct rpc_param *param,
		       void *page_list)
{
	struct tee_shm *shm;

	switch (OPTEE_SMC_RETURN_GET_RPC_FUNC(param->a0)) {
	case OPTEE_SMC_RPC_FUNC_ALLOC:
		if (!__tee_shm_add(dev, OPTEE_MSG_NONCONTIG_PAGE_SIZE, NULL,
				   param->a1, TEE_SHM_ALLOC | TEE_SHM_REGISTER,
				   &shm)) {
			reg_pair_from_64(&param->a1, &param->a2,
					 virt_to_phys(shm->addr));
			/* "cookie" */
			reg_pair_from_64(&param->a4, &param->a5, (ulong)shm);
		} else {
			param->a1 = 0;
			param->a2 = 0;
			param->a4 = 0;
			param->a5 = 0;
		}
		break;
	case OPTEE_SMC_RPC_FUNC_FREE:
		shm = reg_pair_to_ptr(param->a1, param->a2);
		tee_shm_free(shm);
		break;
	case OPTEE_SMC_RPC_FUNC_FOREIGN_INTR:
		break;
	case OPTEE_SMC_RPC_FUNC_CMD:
		shm = reg_pair_to_ptr(param->a1, param->a2);
		optee_suppl_cmd(dev, shm, page_list);
		break;
	default:
		break;
	}

	param->a0 = OPTEE_SMC_CALL_RETURN_FROM_RPC;
}

static u32 call_err_to_res(u32 call_err)
{
	switch (call_err) {
	case OPTEE_SMC_RETURN_OK:
		return TEE_SUCCESS;
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

static void flush_shm_dcache(struct udevice *dev, struct optee_msg_arg *arg)
{
	size_t sz = OPTEE_MSG_GET_ARG_SIZE(arg->num_params);

	flush_dcache_range(rounddown((ulong)arg, CONFIG_SYS_CACHELINE_SIZE),
			   roundup((ulong)arg + sz, CONFIG_SYS_CACHELINE_SIZE));

	tee_flush_all_shm_dcache(dev);
}

static u32 do_call_with_arg(struct udevice *dev, struct optee_msg_arg *arg)
{
	struct optee_pdata *pdata = dev_get_plat(dev);
	struct rpc_param param = { .a0 = OPTEE_SMC_CALL_WITH_ARG };
	void *page_list = NULL;

	reg_pair_from_64(&param.a1, &param.a2, virt_to_phys(arg));
	while (true) {
		struct arm_smccc_res res;

		/* If cache are off from U-Boot, sync the cache shared with OP-TEE */
		if (!dcache_status())
			flush_shm_dcache(dev, arg);

		pdata->invoke_fn(param.a0, param.a1, param.a2, param.a3,
				 param.a4, param.a5, param.a6, param.a7, &res);

		/* If cache are off from U-Boot, sync the cache shared with OP-TEE */
		if (!dcache_status())
			flush_shm_dcache(dev, arg);

		free(page_list);
		page_list = NULL;

		if (OPTEE_SMC_RETURN_IS_RPC(res.a0)) {
			param.a0 = res.a0;
			param.a1 = res.a1;
			param.a2 = res.a2;
			param.a3 = res.a3;
			handle_rpc(dev, &param, &page_list);
		} else {
			/*
			 * In case we've accessed RPMB to serve an RPC
			 * request we need to restore the previously
			 * selected partition as the caller may expect it
			 * to remain unchanged.
			 */
			optee_suppl_rpmb_release(dev);
			return call_err_to_res(res.a0);
		}
	}
}

static int optee_close_session(struct udevice *dev, u32 session)
{
	int rc;
	struct tee_shm *shm;
	struct optee_msg_arg *msg_arg;

	rc = get_msg_arg(dev, 0, &shm, &msg_arg);
	if (rc)
		return rc;

	msg_arg->cmd = OPTEE_MSG_CMD_CLOSE_SESSION;
	msg_arg->session = session;
	do_call_with_arg(dev, msg_arg);

	tee_shm_free(shm);

	return 0;
}

static int optee_open_session(struct udevice *dev,
			      struct tee_open_session_arg *arg,
			      uint num_params, struct tee_param *params)
{
	int rc;
	struct tee_shm *shm;
	struct optee_msg_arg *msg_arg;

	rc = get_msg_arg(dev, num_params + 2, &shm, &msg_arg);
	if (rc)
		return rc;

	msg_arg->cmd = OPTEE_MSG_CMD_OPEN_SESSION;
	/*
	 * Initialize and add the meta parameters needed when opening a
	 * session.
	 */
	msg_arg->params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT |
				  OPTEE_MSG_ATTR_META;
	msg_arg->params[1].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT |
				  OPTEE_MSG_ATTR_META;
	memcpy(&msg_arg->params[0].u.value, arg->uuid, sizeof(arg->uuid));
	memcpy(&msg_arg->params[1].u.value, arg->uuid, sizeof(arg->clnt_uuid));
	msg_arg->params[1].u.value.c = arg->clnt_login;

	rc = to_msg_param(msg_arg->params + 2, num_params, params);
	if (rc)
		goto out;

	arg->ret = do_call_with_arg(dev, msg_arg);
	if (arg->ret) {
		arg->ret_origin = TEE_ORIGIN_COMMS;
		goto out;
	}

	if (from_msg_param(params, num_params, msg_arg->params + 2)) {
		arg->ret = TEE_ERROR_COMMUNICATION;
		arg->ret_origin = TEE_ORIGIN_COMMS;
		/* Close session again to avoid leakage */
		optee_close_session(dev, msg_arg->session);
		goto out;
	}

	arg->session = msg_arg->session;
	arg->ret = msg_arg->ret;
	arg->ret_origin = msg_arg->ret_origin;
out:
	tee_shm_free(shm);

	return rc;
}

static int optee_invoke_func(struct udevice *dev, struct tee_invoke_arg *arg,
			     uint num_params, struct tee_param *params)
{
	struct tee_shm *shm;
	struct optee_msg_arg *msg_arg;
	int rc;

	rc = get_msg_arg(dev, num_params, &shm, &msg_arg);
	if (rc)
		return rc;
	msg_arg->cmd = OPTEE_MSG_CMD_INVOKE_COMMAND;
	msg_arg->func = arg->func;
	msg_arg->session = arg->session;

	rc = to_msg_param(msg_arg->params, num_params, params);
	if (rc)
		goto out;

	arg->ret = do_call_with_arg(dev, msg_arg);
	if (arg->ret) {
		arg->ret_origin = TEE_ORIGIN_COMMS;
		goto out;
	}

	if (from_msg_param(params, num_params, msg_arg->params)) {
		arg->ret = TEE_ERROR_COMMUNICATION;
		arg->ret_origin = TEE_ORIGIN_COMMS;
		goto out;
	}

	arg->ret = msg_arg->ret;
	arg->ret_origin = msg_arg->ret_origin;
out:
	tee_shm_free(shm);
	return rc;
}

static int optee_shm_register(struct udevice *dev, struct tee_shm *shm)
{
	struct tee_shm *shm_arg;
	struct optee_msg_arg *msg_arg;
	void *pl;
	u64 ph_ptr;
	int rc;

	rc = get_msg_arg(dev, 1, &shm_arg, &msg_arg);
	if (rc)
		return rc;

	pl = optee_alloc_and_init_page_list(shm->addr, shm->size, &ph_ptr);
	if (!pl) {
		rc = -ENOMEM;
		goto out;
	}

	msg_arg->cmd = OPTEE_MSG_CMD_REGISTER_SHM;
	msg_arg->params->attr = OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT |
				OPTEE_MSG_ATTR_NONCONTIG;
	msg_arg->params->u.tmem.buf_ptr = ph_ptr;
	msg_arg->params->u.tmem.shm_ref = (ulong)shm;
	msg_arg->params->u.tmem.size = shm->size;

	if (do_call_with_arg(dev, msg_arg) || msg_arg->ret)
		rc = -EINVAL;

	free(pl);
out:
	tee_shm_free(shm_arg);

	return rc;
}

static int optee_shm_unregister(struct udevice *dev, struct tee_shm *shm)
{
	struct tee_shm *shm_arg;
	struct optee_msg_arg *msg_arg;
	int rc;

	rc = get_msg_arg(dev, 1, &shm_arg, &msg_arg);
	if (rc)
		return rc;

	msg_arg->cmd = OPTEE_MSG_CMD_UNREGISTER_SHM;
	msg_arg->params[0].attr = OPTEE_MSG_ATTR_TYPE_RMEM_INPUT;
	msg_arg->params[0].u.rmem.shm_ref = (ulong)shm;

	if (do_call_with_arg(dev, msg_arg) || msg_arg->ret)
		rc = -EINVAL;
	tee_shm_free(shm_arg);

	return rc;
}

static const struct tee_driver_ops optee_ops = {
	.get_version = optee_get_version,
	.open_session = optee_open_session,
	.close_session = optee_close_session,
	.invoke_func = optee_invoke_func,
	.shm_register = optee_shm_register,
	.shm_unregister = optee_shm_unregister,
};

static bool is_optee_api(optee_invoke_fn *invoke_fn)
{
	struct arm_smccc_res res;

	invoke_fn(OPTEE_SMC_CALLS_UID, 0, 0, 0, 0, 0, 0, 0, &res);

	return res.a0 == OPTEE_MSG_UID_0 && res.a1 == OPTEE_MSG_UID_1 &&
	       res.a2 == OPTEE_MSG_UID_2 && res.a3 == OPTEE_MSG_UID_3;
}

static void print_os_revision(struct udevice *dev, optee_invoke_fn *invoke_fn)
{
	union {
		struct arm_smccc_res smccc;
		struct optee_smc_call_get_os_revision_result result;
	} res = {
		.result = {
		.build_id = 0
		}
	};

	invoke_fn(OPTEE_SMC_CALL_GET_OS_REVISION, 0, 0, 0, 0, 0, 0, 0,
		  &res.smccc);

	if (res.result.build_id)
		dev_info(dev, "OP-TEE: revision %lu.%lu (%08lx)\n",
			 res.result.major, res.result.minor,
			 res.result.build_id);
	else
		dev_info(dev, "OP-TEE: revision %lu.%lu\n",
			 res.result.major, res.result.minor);
}

static bool api_revision_is_compatible(optee_invoke_fn *invoke_fn)
{
	union {
		struct arm_smccc_res smccc;
		struct optee_smc_calls_revision_result result;
	} res;

	invoke_fn(OPTEE_SMC_CALLS_REVISION, 0, 0, 0, 0, 0, 0, 0, &res.smccc);

	return res.result.major == OPTEE_MSG_REVISION_MAJOR &&
	       (int)res.result.minor >= OPTEE_MSG_REVISION_MINOR;
}

static bool exchange_capabilities(optee_invoke_fn *invoke_fn, u32 *sec_caps)
{
	union {
		struct arm_smccc_res smccc;
		struct optee_smc_exchange_capabilities_result result;
	} res;

	invoke_fn(OPTEE_SMC_EXCHANGE_CAPABILITIES,
		  OPTEE_SMC_NSEC_CAP_UNIPROCESSOR, 0, 0, 0, 0, 0, 0,
		  &res.smccc);

	if (res.result.status != OPTEE_SMC_RETURN_OK)
		return false;

	*sec_caps = res.result.capabilities;

	return true;
}

/* Simple wrapper functions to be able to use a function pointer */
static void optee_smccc_smc(unsigned long a0, unsigned long a1,
			    unsigned long a2, unsigned long a3,
			    unsigned long a4, unsigned long a5,
			    unsigned long a6, unsigned long a7,
			    struct arm_smccc_res *res)
{
	arm_smccc_smc(a0, a1, a2, a3, a4, a5, a6, a7, res);
}

static void optee_smccc_hvc(unsigned long a0, unsigned long a1,
			    unsigned long a2, unsigned long a3,
			    unsigned long a4, unsigned long a5,
			    unsigned long a6, unsigned long a7,
			    struct arm_smccc_res *res)
{
	arm_smccc_hvc(a0, a1, a2, a3, a4, a5, a6, a7, res);
}

static optee_invoke_fn *get_invoke_func(struct udevice *dev)
{
	const char *method;

	debug("optee: looking for conduit method in DT.\n");
	method = ofnode_get_property(dev_ofnode(dev), "method", NULL);
	if (!method) {
		debug("optee: missing \"method\" property\n");
		return ERR_PTR(-ENXIO);
	}

	if (!strcmp("hvc", method))
		return optee_smccc_hvc;
	else if (!strcmp("smc", method))
		return optee_smccc_smc;

	debug("optee: invalid \"method\" property: %s\n", method);
	return ERR_PTR(-EINVAL);
}

static int optee_of_to_plat(struct udevice *dev)
{
	struct optee_pdata *pdata = dev_get_plat(dev);

	pdata->invoke_fn = get_invoke_func(dev);
	if (IS_ERR(pdata->invoke_fn))
		return PTR_ERR(pdata->invoke_fn);

	return 0;
}

static int optee_bind(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_OPTEE_SERVICE_DISCOVERY))
		dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	return 0;
}

static int optee_probe(struct udevice *dev)
{
	struct optee_pdata *pdata = dev_get_plat(dev);
	u32 sec_caps;
	int ret;

	if (!is_optee_api(pdata->invoke_fn)) {
		dev_err(dev, "OP-TEE api uid mismatch\n");
		return -ENOENT;
	}

	print_os_revision(dev, pdata->invoke_fn);

	if (!api_revision_is_compatible(pdata->invoke_fn)) {
		dev_err(dev, "OP-TEE api revision mismatch\n");
		return -ENOENT;
	}

	/*
	 * OP-TEE can use both shared memory via predefined pool or as
	 * dynamic shared memory provided by normal world. To keep things
	 * simple we're only using dynamic shared memory in this driver.
	 */
	if (!exchange_capabilities(pdata->invoke_fn, &sec_caps) ||
	    !(sec_caps & OPTEE_SMC_SEC_CAP_DYNAMIC_SHM)) {
		dev_err(dev, "OP-TEE capabilities mismatch\n");
		return -ENOENT;
	}

	if (IS_ENABLED(CONFIG_OPTEE_SERVICE_DISCOVERY)) {
		ret = bind_service_drivers(dev);
		if (ret)
			dev_warn(dev, "optee service enumeration failed: %d\n", ret);
	} else if (IS_ENABLED(CONFIG_RNG_OPTEE)) {
		/*
		 * Discovery of TAs on the TEE bus is not supported in U-Boot:
		 * only bind the drivers associated to the supported OP-TEE TA
		 */
		ret = device_bind_driver_to_node(dev, "optee-rng", "optee-rng",
						 dev_ofnode(dev), NULL);
		if (ret)
			dev_warn(dev, "optee-rng failed to bind: %d\n", ret);
	}

	return 0;
}

static const struct udevice_id optee_match[] = {
	{ .compatible = "linaro,optee-tz" },
	{},
};

U_BOOT_DRIVER(optee) = {
	.name = "optee",
	.id = UCLASS_TEE,
	.of_match = optee_match,
	.of_to_plat = optee_of_to_plat,
	.probe = optee_probe,
	.bind = optee_bind,
	.ops = &optee_ops,
	.plat_auto	= sizeof(struct optee_pdata),
	.priv_auto	= sizeof(struct optee_private),
};
