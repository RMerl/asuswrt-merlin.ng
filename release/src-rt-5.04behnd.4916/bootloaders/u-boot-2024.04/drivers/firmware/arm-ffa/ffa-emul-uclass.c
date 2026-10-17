// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */
#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <string.h>
#include <asm/global_data.h>
#include <asm/sandbox_arm_ffa.h>
#include <asm/sandbox_arm_ffa_priv.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/errno.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/* The partitions (SPs) table */
static struct ffa_partition_desc sandbox_partitions[SANDBOX_PARTITIONS_CNT] = {
	{
		.info = { .id = SANDBOX_SP1_ID, .exec_ctxt = 0x5687, .properties = 0x89325621 },
		.sp_uuid = {
			.a1 = SANDBOX_SERVICE1_UUID_A1,
			.a2 = SANDBOX_SERVICE1_UUID_A2,
			.a3 = SANDBOX_SERVICE1_UUID_A3,
			.a4 = SANDBOX_SERVICE1_UUID_A4,
		}
	},
	{
		.info = { .id = SANDBOX_SP3_ID, .exec_ctxt = 0x7687, .properties = 0x23325621 },
		.sp_uuid = {
			.a1 = SANDBOX_SERVICE2_UUID_A1,
			.a2 = SANDBOX_SERVICE2_UUID_A2,
			.a3 = SANDBOX_SERVICE2_UUID_A3,
			.a4 = SANDBOX_SERVICE2_UUID_A4,
		}
	},
	{
		.info = { .id = SANDBOX_SP2_ID, .exec_ctxt = 0x9587, .properties = 0x45325621 },
		.sp_uuid = {
			.a1 = SANDBOX_SERVICE1_UUID_A1,
			.a2 = SANDBOX_SERVICE1_UUID_A2,
			.a3 = SANDBOX_SERVICE1_UUID_A3,
			.a4 = SANDBOX_SERVICE1_UUID_A4,
		}
	},
	{
		.info = { .id = SANDBOX_SP4_ID, .exec_ctxt = 0x1487, .properties = 0x70325621 },
		.sp_uuid = {
			.a1 = SANDBOX_SERVICE2_UUID_A1,
			.a2 = SANDBOX_SERVICE2_UUID_A2,
			.a3 = SANDBOX_SERVICE2_UUID_A3,
			.a4 = SANDBOX_SERVICE2_UUID_A4,
		}
	}

};

/* The emulator functions */

/**
 * sandbox_ffa_version() - Emulated FFA_VERSION handler function
 * @emul: The sandbox FF-A emulator device
 * @pargs: The SMC call input arguments a0-a7
 * @res:  The SMC return data
 *
 * Emulate FFA_VERSION FF-A function.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */

static int sandbox_ffa_version(struct udevice *emul, ffa_value_t *pargs, ffa_value_t *res)
{
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	priv->fwk_version = FFA_VERSION_1_0;
	res->a0 = priv->fwk_version;

	/* x1-x7 MBZ */
	memset(FFA_X1X7_MBZ_REG_START, 0, FFA_X1X7_MBZ_CNT * sizeof(ulong));

	return 0;
}

/**
 * sandbox_ffa_id_get() - Emulated FFA_ID_GET handler function
 * @emul: The sandbox FF-A emulator device
 * @pargs: The SMC call input arguments a0-a7
 * @res:  The SMC return data
 *
 * Emulate FFA_ID_GET FF-A function.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_id_get(struct udevice *emul, ffa_value_t *pargs, ffa_value_t *res)
{
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	res->a0 = FFA_SMC_32(FFA_SUCCESS);
	res->a1 = 0;

	priv->id = NS_PHYS_ENDPOINT_ID;
	res->a2 = priv->id;

	/* x3-x7 MBZ */
	memset(FFA_X3_MBZ_REG_START, 0, FFA_X3X7_MBZ_CNT * sizeof(ulong));

	return 0;
}

/**
 * sandbox_ffa_features() - Emulated FFA_FEATURES handler function
 * @pargs: The SMC call input arguments a0-a7
 * @res:  The SMC return data
 *
 * Emulate FFA_FEATURES FF-A function.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_features(ffa_value_t *pargs, ffa_value_t *res)
{
	res->a1 = 0;

	if (pargs->a1 == FFA_SMC_64(FFA_RXTX_MAP)) {
		res->a0 = FFA_SMC_32(FFA_SUCCESS);
		res->a2 = RXTX_BUFFERS_MIN_SIZE;
		res->a3 = 0;
		/* x4-x7 MBZ */
		memset(FFA_X4X7_MBZ_REG_START, 0, FFA_X4X7_MBZ_CNT * sizeof(ulong));
		return 0;
	}

	res->a0 = FFA_SMC_32(FFA_ERROR);
	res->a2 = -NOT_SUPPORTED;
	/* x3-x7 MBZ */
	memset(FFA_X3_MBZ_REG_START, 0, FFA_X3X7_MBZ_CNT * sizeof(ulong));
	log_err("FF-A interface %lx not implemented\n", pargs->a1);

	return ffa_to_std_errmap[NOT_SUPPORTED];
}

/**
 * sandbox_ffa_partition_info_get() - Emulated FFA_PARTITION_INFO_GET handler
 * @emul: The sandbox FF-A emulator device
 * @pargs: The SMC call input arguments a0-a7
 * @res:  The SMC return data
 *
 * Emulate FFA_PARTITION_INFO_GET FF-A function.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_partition_info_get(struct udevice *emul, ffa_value_t *pargs,
					  ffa_value_t *res)
{
	struct ffa_partition_info *rxbuf_desc_info = NULL;
	u32 descs_cnt;
	u32 descs_size_bytes;
	int ret;
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	res->a0 = FFA_SMC_32(FFA_ERROR);

	if (!priv->pair.rxbuf) {
		res->a2 = -DENIED;
		ret = ffa_to_std_errmap[DENIED];
		goto cleanup;
	}

	if (priv->pair_info.rxbuf_owned) {
		res->a2 = -BUSY;
		ret = ffa_to_std_errmap[BUSY];
		goto cleanup;
	}

	if (!priv->partitions.descs) {
		priv->partitions.descs = sandbox_partitions;
		priv->partitions.count = SANDBOX_PARTITIONS_CNT;
	}

	descs_size_bytes = SANDBOX_PARTITIONS_CNT *
		sizeof(struct ffa_partition_desc);

	/* Abort if the RX buffer size is smaller than the descs buffer size */
	if ((priv->pair_info.rxtx_buf_size * SZ_4K) < descs_size_bytes) {
		res->a2 = -NO_MEMORY;
		ret = ffa_to_std_errmap[NO_MEMORY];
		goto cleanup;
	}

	rxbuf_desc_info = priv->pair.rxbuf;

	/* No UUID specified. Return the information of all partitions */
	if (!pargs->a1 && !pargs->a2 && !pargs->a3 && !pargs->a4) {
		for (descs_cnt = 0; descs_cnt < SANDBOX_PARTITIONS_CNT; descs_cnt++)
			*(rxbuf_desc_info++) = priv->partitions.descs[descs_cnt].info;

		res->a0 = FFA_SMC_32(FFA_SUCCESS);
		res->a2 = SANDBOX_PARTITIONS_CNT;
		/* Transfer ownership to the consumer: the non secure world */
		priv->pair_info.rxbuf_owned = 1;
		ret = 0;

		goto cleanup;
	}

	/* A UUID specified. Return the info of all SPs matching the UUID */

	for (descs_cnt = 0 ; descs_cnt < SANDBOX_PARTITIONS_CNT ; descs_cnt++)
		if (pargs->a1 == priv->partitions.descs[descs_cnt].sp_uuid.a1 &&
		    pargs->a2 == priv->partitions.descs[descs_cnt].sp_uuid.a2 &&
		    pargs->a3 == priv->partitions.descs[descs_cnt].sp_uuid.a3 &&
		    pargs->a4 == priv->partitions.descs[descs_cnt].sp_uuid.a4) {
			*(rxbuf_desc_info++) = priv->partitions.descs[descs_cnt].info;
		}

	if (rxbuf_desc_info != priv->pair.rxbuf) {
		res->a0 = FFA_SMC_32(FFA_SUCCESS);
		/* Store the partitions count */
		res->a2 = (ulong)
			(rxbuf_desc_info - (struct ffa_partition_info *)
			 priv->pair.rxbuf);
		ret = 0;

		/* Transfer ownership to the consumer: the non secure world */
		priv->pair_info.rxbuf_owned = 1;
	} else {
		/* Unrecognized UUID */
		res->a2 = -INVALID_PARAMETERS;
		ret = ffa_to_std_errmap[INVALID_PARAMETERS];
	}

cleanup:

	log_err("FFA_PARTITION_INFO_GET (%ld)\n", res->a2);

	res->a1 = 0;

	/* x3-x7 MBZ */
	memset(FFA_X3_MBZ_REG_START, 0, FFA_X3X7_MBZ_CNT * sizeof(ulong));

	return ret;
}

/**
 * sandbox_ffa_rxtx_map() - Emulated FFA_RXTX_MAP handler
 * @emul: The sandbox FF-A emulator device
 * @pargs: The SMC call input arguments a0-a7
 * @res:  The SMC return data
 *
 * Emulate FFA_RXTX_MAP FF-A function.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_rxtx_map(struct udevice *emul, ffa_value_t *pargs, ffa_value_t *res)
{
	int ret;
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	res->a0 = FFA_SMC_32(FFA_ERROR);

	if (priv->pair.txbuf && priv->pair.rxbuf) {
		res->a2 = -DENIED;
		ret = ffa_to_std_errmap[DENIED];
		goto feedback;
	}

	if (pargs->a3 >= RXTX_BUFFERS_MIN_PAGES && pargs->a1 && pargs->a2) {
		priv->pair.txbuf = map_sysmem(pargs->a1, 0);
		priv->pair.rxbuf = map_sysmem(pargs->a2, 0);
		priv->pair_info.rxtx_buf_size = pargs->a3;
		priv->pair_info.rxbuf_mapped = 1;
		res->a0 = FFA_SMC_32(FFA_SUCCESS);
		res->a2 = 0;
		ret = 0;
		goto feedback;
	}

	if (!pargs->a1 || !pargs->a2) {
		res->a2 = -INVALID_PARAMETERS;
		ret = ffa_to_std_errmap[INVALID_PARAMETERS];
	} else {
		res->a2 = -NO_MEMORY;
		ret = ffa_to_std_errmap[NO_MEMORY];
	}

	log_err("Error in FFA_RXTX_MAP arguments (%d)\n",
		(int)res->a2);

feedback:

	res->a1 = 0;

	/* x3-x7 MBZ */
	memset(FFA_X3_MBZ_REG_START, 0, FFA_X3X7_MBZ_CNT * sizeof(ulong));

	return ret;
}

/**
 * sandbox_ffa_rxtx_unmap() - Emulated FFA_RXTX_UNMAP handler
 * @emul: The sandbox FF-A emulator device
 * @pargs: The SMC call input arguments a0-a7
 * @res:  The SMC return data
 *
 * Emulate FFA_RXTX_UNMAP FF-A function.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_rxtx_unmap(struct udevice *emul, ffa_value_t *pargs, ffa_value_t *res)
{
	int ret;
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	res->a0 = FFA_SMC_32(FFA_ERROR);
	res->a2 = -INVALID_PARAMETERS;
	ret = ffa_to_std_errmap[INVALID_PARAMETERS];

	if (GET_NS_PHYS_ENDPOINT_ID(pargs->a1) != priv->id)
		goto feedback;

	if (priv->pair.txbuf && priv->pair.rxbuf) {
		priv->pair.txbuf = 0;
		priv->pair.rxbuf = 0;
		priv->pair_info.rxtx_buf_size = 0;
		priv->pair_info.rxbuf_mapped = 0;
		res->a0 = FFA_SMC_32(FFA_SUCCESS);
		res->a2 = 0;
		ret = 0;
		goto feedback;
	}

	log_err("No buffer pair registered on behalf of the caller\n");

feedback:

	res->a1 = 0;

	/* x3-x7 MBZ */
	memset(FFA_X3_MBZ_REG_START, 0, FFA_X3X7_MBZ_CNT * sizeof(ulong));

	return ret;
}

/**
 * sandbox_ffa_rx_release() - Emulated FFA_RX_RELEASE handler
 * @emul: The sandbox FF-A emulator device
 * @pargs: The SMC call input arguments a0-a7
 * @res:  The SMC return data
 *
 * Emulate FFA_RX_RELEASE FF-A function.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_rx_release(struct udevice *emul, ffa_value_t *pargs, ffa_value_t *res)
{
	int ret;
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	if (!priv->pair_info.rxbuf_owned) {
		res->a0 = FFA_SMC_32(FFA_ERROR);
		res->a2 = -DENIED;
		ret = ffa_to_std_errmap[DENIED];
	} else {
		priv->pair_info.rxbuf_owned = 0;
		res->a0 = FFA_SMC_32(FFA_SUCCESS);
		res->a2 = 0;
		ret = 0;
	}

	res->a1 = 0;

	/* x3-x7 MBZ */
	memset(FFA_X3_MBZ_REG_START, 0, FFA_X3X7_MBZ_CNT * sizeof(ulong));

	return ret;
}

/**
 * sandbox_ffa_sp_valid() - Check SP validity
 * @emul: The sandbox FF-A emulator device
 * @part_id:	partition ID to check
 *
 * Search the input ID in the descriptors table.
 *
 * Return:
 *
 * 1 on success (Partition found). Otherwise, failure
 */
static int sandbox_ffa_sp_valid(struct udevice *emul, u16 part_id)
{
	u32 descs_cnt;
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	for (descs_cnt = 0 ; descs_cnt < SANDBOX_PARTITIONS_CNT ; descs_cnt++)
		if (priv->partitions.descs[descs_cnt].info.id == part_id)
			return 1;

	return 0;
}

/**
 * sandbox_ffa_msg_send_direct_req() - Emulated FFA_MSG_SEND_DIRECT_{REQ,RESP} handler
 * @emul: The sandbox FF-A emulator device
 * @pargs: The SMC call input arguments a0-a7
 * @res:  The SMC return data
 *
 * Emulate FFA_MSG_SEND_DIRECT_{REQ,RESP} FF-A ABIs.
 * Only SMC 64-bit is supported in Sandbox.
 *
 * Emulating interrupts is not supported. So, FFA_RUN and FFA_INTERRUPT are not
 * supported. In case of success FFA_MSG_SEND_DIRECT_RESP is returned with
 * default pattern data (0xff).
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_msg_send_direct_req(struct udevice *emul,
					   ffa_value_t *pargs, ffa_value_t *res)
{
	u16 part_id;
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	part_id = GET_DST_SP_ID(pargs->a1);

	if (GET_NS_PHYS_ENDPOINT_ID(pargs->a1) != priv->id ||
	    !sandbox_ffa_sp_valid(emul, part_id) || pargs->a2) {
		res->a0 = FFA_SMC_32(FFA_ERROR);
		res->a1 = 0;
		res->a2 = -INVALID_PARAMETERS;

		/* x3-x7 MBZ */
		memset(FFA_X3_MBZ_REG_START, 0, FFA_X3X7_MBZ_CNT * sizeof(ulong));

		return ffa_to_std_errmap[INVALID_PARAMETERS];
	}

	res->a0 = FFA_SMC_64(FFA_MSG_SEND_DIRECT_RESP);

	res->a1 = PREP_SRC_SP_ID(part_id) |
		PREP_NS_PHYS_ENDPOINT_ID(priv->id);

	res->a2 = 0;

	/* Return 0xff bytes as a response */
	res->a3 = -1UL;
	res->a4 = -1UL;
	res->a5 = -1UL;
	res->a6 = -1UL;
	res->a7 = -1UL;

	return 0;
}

/**
 * sandbox_ffa_get_rxbuf_flags() - Read the mapping/ownership flags
 * @emul: The sandbox FF-A emulator device
 * @queried_func_id:	The FF-A function to be queried
 * @func_data:  Pointer to the FF-A function arguments container structure
 *
 * Query the status flags of the following emulated
 * ABIs: FFA_RXTX_MAP, FFA_RXTX_UNMAP, FFA_RX_RELEASE.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_get_rxbuf_flags(struct udevice *emul, u32 queried_func_id,
				       struct ffa_sandbox_data *func_data)
{
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	if (!func_data)
		return -EINVAL;

	if (!func_data->data0 || func_data->data0_size != sizeof(u8))
		return -EINVAL;

	switch (queried_func_id) {
	case FFA_RXTX_MAP:
	case FFA_RXTX_UNMAP:
		*((u8 *)func_data->data0) = priv->pair_info.rxbuf_mapped;
		return 0;
	case FFA_RX_RELEASE:
		*((u8 *)func_data->data0) = priv->pair_info.rxbuf_owned;
		return 0;
	default:
		log_err("The querried FF-A interface flag (%d) undefined\n",
			queried_func_id);
		return -EINVAL;
	}
}

/**
 * sandbox_ffa_get_fwk_version() - Return the FFA framework version
 * @emul: The sandbox FF-A emulator device
 * @func_data:  Pointer to the FF-A function arguments container structure
 *
 * Return the FFA framework version read from the FF-A emulator data.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_get_fwk_version(struct udevice *emul, struct ffa_sandbox_data *func_data)
{
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	if (!func_data)
		return -EINVAL;

	if (!func_data->data0 ||
	    func_data->data0_size != sizeof(priv->fwk_version))
		return -EINVAL;

	*((u32 *)func_data->data0) = priv->fwk_version;

	return 0;
}

/**
 * sandbox_ffa_get_parts() - Return the address of partitions data
 * @emul: The sandbox FF-A emulator device
 * @func_data:  Pointer to the FF-A function arguments container structure
 *
 * Return the address of partitions data read from the FF-A emulator data.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_get_parts(struct udevice *emul, struct ffa_sandbox_data *func_data)
{
	struct sandbox_ffa_emul *priv = dev_get_priv(emul);

	if (!func_data)
		return -EINVAL;

	if (!func_data->data0 ||
	    func_data->data0_size != sizeof(struct ffa_partitions *))
		return -EINVAL;

	*((struct ffa_partitions **)func_data->data0) = &priv->partitions;

	return 0;
}

/**
 * sandbox_query_ffa_emul_state() - Inspect the FF-A ABIs
 * @queried_func_id:	The FF-A function to be queried
 * @func_data:  Pointer to the FF-A function arguments container structure
 *
 * Query the status of FF-A ABI specified in the input argument.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int sandbox_query_ffa_emul_state(u32 queried_func_id,
				 struct ffa_sandbox_data *func_data)
{
	struct udevice *emul;
	int ret;

	ret = uclass_first_device_err(UCLASS_FFA_EMUL, &emul);
	if (ret) {
		log_err("Cannot find FF-A emulator during querying state\n");
		return ret;
	}

	switch (queried_func_id) {
	case FFA_RXTX_MAP:
	case FFA_RXTX_UNMAP:
	case FFA_RX_RELEASE:
		return sandbox_ffa_get_rxbuf_flags(emul, queried_func_id, func_data);
	case FFA_VERSION:
		return sandbox_ffa_get_fwk_version(emul, func_data);
	case FFA_PARTITION_INFO_GET:
		return sandbox_ffa_get_parts(emul, func_data);
	default:
		log_err("Undefined FF-A interface (%d)\n",
			queried_func_id);
		return -EINVAL;
	}
}

/**
 * sandbox_arm_ffa_smccc_smc() - FF-A SMC call emulation
 * @args:	the SMC call arguments
 * @res:	the SMC call returned data
 *
 * Emulate the FF-A ABIs SMC call.
 * The emulated FF-A ABI is identified and invoked.
 * FF-A emulation is based on the FF-A specification 1.0
 *
 * Return:
 *
 * 0 on success. Otherwise, failure.
 * FF-A protocol error codes are returned using the registers arguments as
 * described by the specification
 */
void sandbox_arm_ffa_smccc_smc(ffa_value_t *args, ffa_value_t *res)
{
	int ret = 0;
	struct udevice *emul;

	ret = uclass_first_device_err(UCLASS_FFA_EMUL, &emul);
	if (ret) {
		log_err("Cannot find FF-A emulator during SMC emulation\n");
		return;
	}

	switch (args->a0) {
	case FFA_SMC_32(FFA_VERSION):
		ret = sandbox_ffa_version(emul, args, res);
		break;
	case FFA_SMC_32(FFA_PARTITION_INFO_GET):
		ret = sandbox_ffa_partition_info_get(emul, args, res);
		break;
	case FFA_SMC_32(FFA_RXTX_UNMAP):
		ret = sandbox_ffa_rxtx_unmap(emul, args, res);
		break;
	case FFA_SMC_64(FFA_MSG_SEND_DIRECT_REQ):
		ret = sandbox_ffa_msg_send_direct_req(emul, args, res);
		break;
	case FFA_SMC_32(FFA_ID_GET):
		ret = sandbox_ffa_id_get(emul, args, res);
		break;
	case FFA_SMC_32(FFA_FEATURES):
		ret = sandbox_ffa_features(args, res);
		break;
	case FFA_SMC_64(FFA_RXTX_MAP):
		ret = sandbox_ffa_rxtx_map(emul, args, res);
		break;
	case FFA_SMC_32(FFA_RX_RELEASE):
		ret = sandbox_ffa_rx_release(emul, args, res);
		break;
	default:
		log_err("Undefined FF-A interface (%lx)\n",
			args->a0);
	}

	if (ret != 0)
		log_err("FF-A ABI internal failure (%d)\n", ret);
}

/**
 * invoke_ffa_fn() - SMC wrapper
 * @args: FF-A ABI arguments to be copied to Xn registers
 * @res: FF-A ABI return data to be copied from Xn registers
 *
 * Calls the emulated SMC call.
 */
void invoke_ffa_fn(ffa_value_t args, ffa_value_t *res)
{
	sandbox_arm_ffa_smccc_smc(&args, res);
}

/**
 * ffa_emul_find() - Find the FF-A emulator
 * @dev:	the sandbox FF-A device (sandbox-arm-ffa)
 * @emulp:	the FF-A emulator device (sandbox-ffa-emul)
 *
 * Search for the FF-A emulator and returns its device pointer.
 *
 * Return:
 * 0 on success. Otherwise, failure
 */
int ffa_emul_find(struct udevice *dev, struct udevice **emulp)
{
	int ret;

	ret = uclass_first_device_err(UCLASS_FFA_EMUL, emulp);
	if (ret) {
		log_err("Cannot find FF-A emulator\n");
		return ret;
	}

	log_debug("FF-A emulator ready to use\n");

	return 0;
}

UCLASS_DRIVER(ffa_emul) = {
	.name		= "ffa_emul",
	.id		= UCLASS_FFA_EMUL,
	.post_bind = dm_scan_fdt_dev,
};

static const struct udevice_id sandbox_ffa_emul_ids[] = {
	{ .compatible = "sandbox,arm-ffa-emul" },
	{ }
};

/* Declaring the sandbox FF-A emulator under UCLASS_FFA_EMUL */
U_BOOT_DRIVER(sandbox_ffa_emul) = {
	.name		= "sandbox_ffa_emul",
	.id		= UCLASS_FFA_EMUL,
	.of_match	= sandbox_ffa_emul_ids,
	.priv_auto	= sizeof(struct sandbox_ffa_emul),
};
