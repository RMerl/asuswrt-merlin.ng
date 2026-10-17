// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <arm_ffa.h>
#include <arm_ffa_priv.h>
#include <dm.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * invoke_ffa_fn() - SMC wrapper
 * @args: FF-A ABI arguments to be copied to Xn registers
 * @res: FF-A ABI return data to be copied from Xn registers
 *
 * Calls low level SMC assembly function
 */
void invoke_ffa_fn(ffa_value_t args, ffa_value_t *res)
{
	arm_smccc_1_2_smc(&args, res);
}

/**
 * arm_ffa_discover() - perform FF-A discovery
 * @dev: The Arm FF-A bus device (arm_ffa)
 * Try to discover the FF-A framework. Discovery is performed by
 * querying the FF-A framework version from secure world using the FFA_VERSION ABI.
 * Return:
 *
 * true on success. Otherwise, false.
 */
static bool arm_ffa_discover(struct udevice *dev)
{
	int ret;

	log_debug("Arm FF-A framework discovery\n");

	ret = ffa_get_version_hdlr(dev);
	if (ret)
		return false;

	return true;
}

/**
 * arm_ffa_is_supported() - FF-A bus discovery callback
 * @invoke_fn: legacy SMC invoke function (not used)
 *
 * Perform FF-A discovery by calling arm_ffa_discover().
 * Discovery is performed by querying the FF-A framework version from
 * secure world using the FFA_VERSION ABI.
 *
 * The FF-A driver is registered as an SMCCC feature driver. So, features discovery
 * callbacks are called by the PSCI driver (PSCI device is the SMCCC features
 * root device).
 *
 * The FF-A driver supports the SMCCCv1.2 extended input/output registers.
 * So, the legacy SMC invocation is not used.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static bool arm_ffa_is_supported(void (*invoke_fn)(ulong a0, ulong a1,
						   ulong a2, ulong a3,
						   ulong a4, ulong a5,
						   ulong a6, ulong a7,
						   struct arm_smccc_res *res))
{
	return arm_ffa_discover(NULL);
}

/* Arm FF-A driver operations */

static const struct ffa_bus_ops ffa_ops = {
	.partition_info_get = ffa_get_partitions_info_hdlr,
	.sync_send_receive = ffa_msg_send_direct_req_hdlr,
	.rxtx_unmap = ffa_unmap_rxtx_buffers_hdlr,
};

/* Registering the FF-A driver as an SMCCC feature driver */

ARM_SMCCC_FEATURE_DRIVER(arm_ffa) = {
	.driver_name = FFA_DRV_NAME,
	.is_supported = arm_ffa_is_supported,
};

/* Declaring the FF-A driver under UCLASS_FFA */

U_BOOT_DRIVER(arm_ffa) = {
	.name		= FFA_DRV_NAME,
	.id		= UCLASS_FFA,
	.flags		= DM_REMOVE_OS_PREPARE,
	.ops		= &ffa_ops,
};
