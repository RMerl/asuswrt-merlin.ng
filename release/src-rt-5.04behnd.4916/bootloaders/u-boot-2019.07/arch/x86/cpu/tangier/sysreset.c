// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * Reset driver for tangier processor
 */

#include <common.h>
#include <dm.h>
#include <sysreset.h>
#include <asm/scu.h>

static int tangier_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	int value;

	switch (type) {
	case SYSRESET_WARM:
		value = IPCMSG_WARM_RESET;
		break;
	case SYSRESET_COLD:
		value = IPCMSG_COLD_RESET;
		break;
	default:
		return -ENOSYS;
	}

	scu_ipc_simple_command(value, 0);

	return -EINPROGRESS;
}

static const struct udevice_id tangier_sysreset_ids[] = {
	{ .compatible = "intel,reset-tangier" },
	{ }
};

static struct sysreset_ops tangier_sysreset_ops = {
	.request = tangier_sysreset_request,
};

U_BOOT_DRIVER(tangier_sysreset) = {
	.name = "tangier-sysreset",
	.id = UCLASS_SYSRESET,
	.of_match = tangier_sysreset_ids,
	.ops = &tangier_sysreset_ops,
};
