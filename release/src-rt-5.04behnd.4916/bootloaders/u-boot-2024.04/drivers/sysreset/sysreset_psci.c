// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <dm.h>
#include <sysreset.h>
#include <linux/errno.h>
#include <linux/psci.h>

__weak int psci_sysreset_get_status(struct udevice *dev, char *buf, int size)
{
	return -EOPNOTSUPP;
}

static int psci_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	switch (type) {
	case SYSRESET_WARM:
	case SYSRESET_COLD:
		psci_sys_reset(type);
		break;
	case SYSRESET_POWER_OFF:
		psci_sys_poweroff();
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops psci_sysreset_ops = {
	.request = psci_sysreset_request,
	.get_status = psci_sysreset_get_status,
};

U_BOOT_DRIVER(psci_sysreset) = {
	.name = "psci-sysreset",
	.id = UCLASS_SYSRESET,
	.ops = &psci_sysreset_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
