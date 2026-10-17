// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */
#include <common.h>
#include <arm_ffa.h>
#include <dm.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/sandbox_arm_ffa_priv.h>
#include <dm/device-internal.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * sandbox_ffa_discover() - perform sandbox FF-A discovery
 * @dev: The sandbox FF-A bus device
 * Try to discover the FF-A framework. Discovery is performed by
 * querying the FF-A framework version from secure world using the FFA_VERSION ABI.
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int sandbox_ffa_discover(struct udevice *dev)
{
	int ret;
	struct udevice *emul;

	log_debug("Emulated FF-A framework discovery\n");

	ret = ffa_emul_find(dev, &emul);
	if (ret) {
		log_err("Cannot find FF-A emulator\n");
		return ret;
	}

	ret = ffa_get_version_hdlr(dev);
	if (ret)
		return ret;

	return 0;
}

/**
 * sandbox_ffa_probe() - The sandbox FF-A driver probe function
 * @dev:	the sandbox-arm-ffa device
 * Save the emulator device in uc_priv.
 * Return:
 *
 * 0 on success.
 */
static int sandbox_ffa_probe(struct udevice *dev)
{
	int ret;
	struct ffa_priv *uc_priv = dev_get_uclass_priv(dev);

	ret = uclass_first_device_err(UCLASS_FFA_EMUL, &uc_priv->emul);
	if (ret) {
		log_err("Cannot find FF-A emulator\n");
		return ret;
	}

	return 0;
}

/**
 * sandbox_ffa_bind() - The sandbox FF-A driver bind function
 * @dev:	the sandbox-arm-ffa device
 * Try to discover the emulated FF-A bus.
 * Return:
 *
 * 0 on success.
 */
static int sandbox_ffa_bind(struct udevice *dev)
{
	int ret;

	ret = sandbox_ffa_discover(dev);
	if (ret)
		return ret;

	return 0;
}

/* Sandbox Arm FF-A emulator operations */

static const struct ffa_bus_ops sandbox_ffa_ops = {
	.partition_info_get = ffa_get_partitions_info_hdlr,
	.sync_send_receive = ffa_msg_send_direct_req_hdlr,
	.rxtx_unmap = ffa_unmap_rxtx_buffers_hdlr,
};

static const struct udevice_id sandbox_ffa_id[] = {
	{ "sandbox,arm-ffa", 0 },
	{ },
};

/* Declaring the sandbox FF-A driver under UCLASS_FFA */
U_BOOT_DRIVER(sandbox_arm_ffa) = {
	.name		= "sandbox_arm_ffa",
	.of_match = sandbox_ffa_id,
	.id		= UCLASS_FFA,
	.bind		= sandbox_ffa_bind,
	.probe		= sandbox_ffa_probe,
	.ops		= &sandbox_ffa_ops,
};
